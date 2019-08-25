#include "Scene.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include <assimp/camera.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <bx/file.h>
#include <bimg/decode.h>
#include <algorithm>

bx::DefaultAllocator Scene::allocator;

Scene::Scene() :
    loaded(false),
    //skyColor({ 0.53f, 0.81f, 0.98f }), // https://en.wikipedia.org/wiki/Sky_blue#Light_sky_blue
    //skyColor({ 0.1f, 0.1f, 0.44f }),   // https://en.wikipedia.org/wiki/Midnight_blue#X11
    skyColor({ 0.0f, 0.0f, 0.0f }),
    ambientLight({ { 0.03f, 0.03f, 0.03f } })
{
    Assimp::DefaultLogger::set(&logSource);
}

Scene::~Scene()
{
}

void Scene::init()
{
    Mesh::PosNormalTangentTex0Vertex::init();
}

void Scene::clear()
{
    if(loaded)
    {
        for(const Mesh& mesh : meshes)
        {
            bgfx::destroy(mesh.vertexBuffer);
            bgfx::destroy(mesh.indexBuffer);
            //bgfx::destroy(mesh.occlusionQuery);
        }
        meshes.clear();
        for(const Material& mat : materials)
        {
            if(bgfx::isValid(mat.baseColorTexture))
                bgfx::destroy(mat.baseColorTexture);
            if(bgfx::isValid(mat.metallicRoughnessTexture))
                bgfx::destroy(mat.metallicRoughnessTexture);
            if(bgfx::isValid(mat.normalTexture))
                bgfx::destroy(mat.normalTexture);
        }
        materials.clear();
        pointLights.shutdown();
        pointLights.lights.clear();
    }
    minBounds = maxBounds = { 0.0f, 0.0f, 0.0f };
    camera = Camera();
    loaded = false;
}

bool Scene::load(const char* file)
{
    clear();

    pointLights.init();

    Assimp::Importer importer;
    // only take triangles or higher (polygons are triangulated during import)
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

    // TODO? importer.SetProgressHandler

    unsigned int flags = aiProcessPreset_TargetRealtime_Quality | // some optimizations and safety checks
        aiProcess_OptimizeMeshes       | // minimize number of meshes
        aiProcess_PreTransformVertices | // apply node matrices
        aiProcess_FixInfacingNormals   |
        aiProcess_TransformUVCoords    | // apply UV transformations
                                         // something here is wrong, Renderer::blit culls CW, but quad triangles are in CW
        //aiProcess_FlipWindingOrder   | // flag in bgfx::setState(), default is to cull(!) CW
        aiProcess_MakeLeftHanded       | // we set GLM_FORCE_LEFT_HANDED and use left-handed bx matrix functions
        aiProcess_FlipUVs;               // bimg loads textures with flipped Y (top left is 0,0)

    const aiScene* scene = importer.ReadFile(file, flags);
    if(scene)
    {
        if(!(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
        {
            for(unsigned int i = 0; i < scene->mNumMeshes; i++)
            {
                try
                {
                    meshes.push_back(loadMesh(scene->mMeshes[i]));
                }
                catch(std::exception& e)
                {
                    Log->warn("{}", e.what());
                }
            }

            char dir[bx::kMaxFilePath] = "";
            bx::strCopy(dir, BX_COUNTOF(dir), bx::FilePath(file).getPath());
            for(unsigned int i = 0; i < scene->mNumMaterials; i++)
            {
                try
                {
                    materials.push_back(loadMaterial(scene->mMaterials[i], dir));
                }
                catch(std::exception& e)
                {
                    // material not loaded, use default
                    // really only happens if there is no diffuse color
                    materials.push_back(Material());
                    Log->warn("{}", e.what());
                }
            }

            // bring opaque meshes to the front so alpha blending works
            // still need depth sorting for scenes with overlapping transparent meshes
            std::partition(meshes.begin(), meshes.end(), [this](const Mesh& mesh) {
                return !materials[mesh.material].blend;
            });

            if(scene->HasCameras())
            {
                camera = loadCamera(scene->mCameras[0]);
            }
            else
            {
                Log->info("No camera, using default");

                // choose appropriate camera planes
                glm::vec3 extent = glm::abs(maxBounds - minBounds);
                float diagonal = glm::sqrt(glm::dot(extent, extent));
                camera.zFar = diagonal;
                camera.zNear = camera.zFar / 50.0f;
            }

            loaded = true;
        }
        else
            Log->error("Scene is incomplete or invalid");
    }

    return loaded;
}

Mesh Scene::loadMesh(const aiMesh* mesh)
{
    if(mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
        throw std::runtime_error("Mesh has incompatible primitive type");

    constexpr size_t coords = 0;
    bool hasTexture = mesh->mNumUVComponents[coords] == 2 && mesh->mTextureCoords[coords] != nullptr;

    // vertices

    uint32_t stride = Mesh::PosNormalTangentTex0Vertex::decl.getStride();

    const bgfx::Memory* vertexMem = bgfx::alloc(mesh->mNumVertices * stride);

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        size_t offset = i * stride;
        Mesh::PosNormalTangentTex0Vertex& vertex = *(Mesh::PosNormalTangentTex0Vertex*)(vertexMem->data + offset);

        aiVector3D pos = mesh->mVertices[i];
        vertex.x = pos.x;
        vertex.y = pos.y;
        vertex.z = pos.z;

        minBounds = glm::min(minBounds, { pos.x, pos.y, pos.z });
        maxBounds = glm::max(maxBounds, { pos.x, pos.y, pos.z });

        aiVector3D nrm = mesh->mNormals[i];
        vertex.nx = nrm.x;
        vertex.ny = nrm.y;
        vertex.nz = nrm.z;

        aiVector3D tan = mesh->mTangents[i];
        vertex.tx = tan.x;
        vertex.ty = tan.y;
        vertex.tz = tan.z;

        if(hasTexture)
        {
            aiVector3D uv = mesh->mTextureCoords[coords][i];
            vertex.u = uv.x;
            vertex.v = uv.y;
        }
    }

    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(vertexMem, Mesh::PosNormalTangentTex0Vertex::decl);

    // indices (triangles)

    const bgfx::Memory* iMem = bgfx::alloc(mesh->mNumFaces * 3 * sizeof(uint16_t));
    uint16_t* indices = (uint16_t*)iMem->data;

    const aiFace* faces = mesh->mFaces;
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        assert(mesh->mFaces[i].mNumIndices == 3);
        indices[(3 * i) + 0] = (uint16_t)mesh->mFaces[i].mIndices[0];
        indices[(3 * i) + 1] = (uint16_t)mesh->mFaces[i].mIndices[1];
        indices[(3 * i) + 2] = (uint16_t)mesh->mFaces[i].mIndices[2];
    }

    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(iMem);

    //bgfx::OcclusionQueryHandle oq = bgfx::createOcclusionQuery();

    return { vbh, ibh, mesh->mMaterialIndex/*, oq*/ };
}

Material Scene::loadMaterial(const aiMaterial* material, const char* dir)
{
    Material out;

    // technically there is a difference between MASK and BLEND mode
    // but for our purposes it's enough if we sort properly
    aiString alphaMode;
    material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    aiString alphaModeOpaque;
    alphaModeOpaque.Set("OPAQUE");
    out.blend = alphaMode != alphaModeOpaque;

    material->Get(AI_MATKEY_TWOSIDED, out.doubleSided);

    aiString fileBaseColor, fileMetallicRoughness, fileNormals;
    material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &fileBaseColor);
    material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &fileMetallicRoughness);
    material->GetTexture(aiTextureType_NORMALS, 0, &fileNormals);

    if(fileBaseColor.length > 0)
    {
        aiString pathBaseColor;
        pathBaseColor.Set(dir);
        pathBaseColor.Append(fileBaseColor.C_Str());
        out.baseColorTexture = loadTexture(pathBaseColor.C_Str());
    }
    else
    {
        Log->warn("Material has no PBR base color texture");
        aiColor4D baseColorFactor;
        if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, baseColorFactor))
            out.baseColorFactor = { baseColorFactor.r, baseColorFactor.g, baseColorFactor.b, baseColorFactor.a };
        else
            throw std::runtime_error("Material has no PBR base color");
        out.baseColorFactor = glm::clamp(out.baseColorFactor, 0.0f, 1.0f);
    }

    if(fileMetallicRoughness.length > 0)
    {
        aiString pathMetallicRoughness;
        pathMetallicRoughness.Set(dir);
        pathMetallicRoughness.Append(fileMetallicRoughness.C_Str());
        out.metallicRoughnessTexture = loadTexture(pathMetallicRoughness.C_Str());
    }
    else
    {
        Log->warn("Material has no PBR metallic/roughness texture");

        ai_real metallicFactor;
        if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallicFactor))
            out.metallicFactor = glm::clamp(metallicFactor, 0.0f, 1.0f);
        else
            Log->warn("Material has no PBR metallic factor, using default of ", out.metallicFactor);

        ai_real roughnessFactor;
        if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughnessFactor))
            out.roughnessFactor = glm::clamp(roughnessFactor, 0.0f, 1.0f);
        else
            Log->warn("Material has no PBR roughness factor, using default of ", out.roughnessFactor);
    }

    if(fileNormals.length > 0)
    {
        aiString pathNormals;
        pathNormals.Set(dir);
        pathNormals.Append(fileNormals.C_Str());
        out.normalTexture = loadTexture(pathNormals.C_Str());
    }

    return out;
}

Camera Scene::loadCamera(const aiCamera* camera)
{
    float aspect = camera->mAspect == 0.0f
                       ? 16.0f/9.0f
                       : camera->mAspect;
    glm::vec3 pos(camera->mPosition.x, camera->mPosition.y, camera->mPosition.z);
    glm::vec3 target(camera->mLookAt.x, camera->mLookAt.y, camera->mLookAt.z);
    glm::vec3 up(camera->mUp.x, camera->mUp.y, camera->mUp.z);

    Camera cam;
    cam.lookAt(pos, target, up);
    // convert horizontal half angle (radians) to vertical full angle (degrees)
    cam.fov = glm::degrees(2.0f * glm::atan(glm::tan(camera->mHorizontalFOV) / aspect));
    cam.zNear = camera->mClipPlaneNear;
    cam.zFar  = camera->mClipPlaneFar;

    return cam;
}

bgfx::TextureHandle Scene::loadTexture(const char* file)
{
    void* data = nullptr;
    uint32_t size = 0;

    bx::FileReader reader;
    bx::Error err;
    if(bx::open(&reader, file, &err))
    {
        size = (uint32_t)bx::getSize(&reader);
        data = BX_ALLOC(&allocator, size);
        bx::read(&reader, data, size, &err);
        bx::close(&reader);
    }

    if(!err.isOk())
    {
        BX_FREE(&allocator, data);
        throw std::runtime_error(err.getMessage().getPtr());
    }

    bimg::ImageContainer* image = bimg::imageParse(&allocator, data, size);
    if(image)
    {
        // the callback gets called when bgfx is done using the data (after 2 frames)
        const bgfx::Memory* mem = bgfx::makeRef(image->m_data, image->m_size, [](void*, void* data) {
            bimg::imageFree((bimg::ImageContainer*)data);
        }, image);
        BX_FREE(&allocator, data);

        if(bgfx::isTextureValid(0, false, image->m_numLayers, (bgfx::TextureFormat::Enum)image->m_format, BGFX_TEXTURE_NONE))
        {
            bgfx::TextureHandle tex = bgfx::createTexture2D((uint16_t)image->m_width,
                                                            (uint16_t)image->m_height,
                                                            image->m_numMips > 1,
                                                            image->m_numLayers,
                                                            (bgfx::TextureFormat::Enum)image->m_format,
                                                            BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC,
                                                            mem);
            //bgfx::setName(tex, file); // causes debug errors with DirectX SetPrivateProperty duplicate
            return tex;
        }
        else
            throw std::runtime_error("Unsupported image format");
    }

    BX_FREE(&allocator, data);
    throw std::runtime_error(err.getMessage().getPtr());
}
