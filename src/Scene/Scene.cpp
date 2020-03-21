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
    minBounds(0.0f),
    maxBounds(0.0f),
    center(0.0f),
    diagonal(0.0f),
    skyColor({ 0.53f, 0.81f, 0.98f }), // https://en.wikipedia.org/wiki/Sky_blue#Light_sky_blue
    //skyColor({ 0.1f, 0.1f, 0.44f }),   // https://en.wikipedia.org/wiki/Midnight_blue#X11
    //skyColor({ 0.0f, 0.0f, 0.0f }),
    ambientLight({ { 0.03f, 0.03f, 0.03f } })
{
    Assimp::DefaultLogger::set(&logSource);
}

void Scene::init()
{
    Mesh::PosNormalTangentTex0Vertex::init();
}

void Scene::clear()
{
    if(loaded)
    {
        for(Mesh& mesh : meshes)
        {
            bgfx::destroy(mesh.vertexBuffer);
            bgfx::destroy(mesh.indexBuffer);
            mesh.vertexBuffer = BGFX_INVALID_HANDLE;
            mesh.indexBuffer = BGFX_INVALID_HANDLE;
        }

        for(Material& mat : materials)
        {
            if(bgfx::isValid(mat.baseColorTexture))
            {
                bgfx::destroy(mat.baseColorTexture);
                mat.baseColorTexture = BGFX_INVALID_HANDLE;
            }
            if(bgfx::isValid(mat.metallicRoughnessTexture))
            {
                bgfx::destroy(mat.metallicRoughnessTexture);
                mat.metallicRoughnessTexture = BGFX_INVALID_HANDLE;
            }
            if(bgfx::isValid(mat.normalTexture))
            {
                bgfx::destroy(mat.normalTexture);
                mat.normalTexture = BGFX_INVALID_HANDLE;
            }
            if(bgfx::isValid(mat.occlusionTexture))
            {
                bgfx::destroy(mat.occlusionTexture);
                mat.occlusionTexture = BGFX_INVALID_HANDLE;
            }
            if(bgfx::isValid(mat.emissiveTexture))
            {
                bgfx::destroy(mat.emissiveTexture);
                mat.emissiveTexture = BGFX_INVALID_HANDLE;
            }
        }

        meshes.clear();
        materials.clear();
        pointLights.shutdown();
        pointLights.lights.clear();
    }
    minBounds = maxBounds = { 0.0f, 0.0f, 0.0f };
    center = { 0.0f, 0.0f, 0.0f };
    diagonal = 0.0f;
    camera = Camera();
    loaded = false;
}

bool Scene::load(const char* file)
{
    clear();

    pointLights.init();

    Assimp::Importer importer;

    // Settings for aiProcess_SortByPType
    // only take triangles or higher (polygons are triangulated during import)
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    // Settings for aiProcess_SplitLargeMeshes
    // Limit vertices to 65k (we use 16-bit indices)
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT,
                                static_cast<int>(std::numeric_limits<uint16_t>::max()) + 1);

    // TODO? importer.SetProgressHandler

    unsigned int flags =
        aiProcessPreset_TargetRealtime_Quality |                     // some optimizations and safety checks
        aiProcess_OptimizeMeshes |                                   // minimize number of meshes
        aiProcess_PreTransformVertices |                             // apply node matrices
        aiProcess_FixInfacingNormals | aiProcess_TransformUVCoords | // apply UV transformations
        //aiProcess_FlipWindingOrder   | // we cull clock-wise, keep the default CCW winding order
        aiProcess_MakeLeftHanded | // we set GLM_FORCE_LEFT_HANDED and use left-handed bx matrix functions
        aiProcess_FlipUVs;         // bimg loads textures with flipped Y (top left is 0,0)

    const aiScene* scene = nullptr;
    try
    {
        scene = importer.ReadFile(file, flags);
    }
    catch(const std::exception& e)
    {
        Log->error("{}", e.what());
    }

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

            center = minBounds + (maxBounds - minBounds) / 2.0f;
            glm::vec3 extent = glm::abs(maxBounds - minBounds);
            diagonal = glm::sqrt(glm::dot(extent, extent));

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
            std::partition(
                meshes.begin(), meshes.end(), [this](const Mesh& mesh) { return !materials[mesh.material].blend; });

            if(scene->HasCameras())
            {
                camera = loadCamera(scene->mCameras[0]);
            }
            else
            {
                Log->info("No camera");
                camera.lookAt(center - glm::vec3(0.0f, 0.0f, diagonal / 2.0f), center, glm::vec3(0.0f, 1.0f, 0.0f));
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

    // TODO? use 32-bit indices by default
    if(mesh->mNumVertices > (std::numeric_limits<uint16_t>::max() + 1u))
        throw std::runtime_error("Mesh has too many vertices (> uint16_t::max + 1)");

    constexpr size_t coords = 0;
    bool hasTexture = mesh->mNumUVComponents[coords] == 2 && mesh->mTextureCoords[coords] != nullptr;

    // vertices

    uint32_t stride = Mesh::PosNormalTangentTex0Vertex::decl.getStride();

    const bgfx::Memory* vertexMem = bgfx::alloc(mesh->mNumVertices * stride);

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        unsigned int offset = i * stride;
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

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        assert(mesh->mFaces[i].mNumIndices == 3);
        indices[(3 * i) + 0] = (uint16_t)mesh->mFaces[i].mIndices[0];
        indices[(3 * i) + 1] = (uint16_t)mesh->mFaces[i].mIndices[1];
        indices[(3 * i) + 2] = (uint16_t)mesh->mFaces[i].mIndices[2];
    }

    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(iMem);

    return { vbh, ibh, mesh->mMaterialIndex };
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

    // texture files

    aiString fileBaseColor, fileMetallicRoughness, fileNormals, fileOcclusion, fileEmissive;
    material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &fileBaseColor);
    material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &fileMetallicRoughness);
    material->GetTexture(aiTextureType_NORMALS, 0, &fileNormals);
    material->GetTexture(aiTextureType_LIGHTMAP, 0, &fileOcclusion);
    material->GetTexture(aiTextureType_EMISSIVE, 0, &fileEmissive);

    // diffuse

    if(fileBaseColor.length > 0)
    {
        aiString pathBaseColor;
        pathBaseColor.Set(dir);
        pathBaseColor.Append(fileBaseColor.C_Str());
        out.baseColorTexture = loadTexture(pathBaseColor.C_Str(), true /* sRGB */);
    }

    aiColor4D baseColorFactor;
    if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, baseColorFactor))
        out.baseColorFactor = { baseColorFactor.r, baseColorFactor.g, baseColorFactor.b, baseColorFactor.a };
    out.baseColorFactor = glm::clamp(out.baseColorFactor, 0.0f, 1.0f);

    // metallic/roughness

    if(fileMetallicRoughness.length > 0)
    {
        aiString pathMetallicRoughness;
        pathMetallicRoughness.Set(dir);
        pathMetallicRoughness.Append(fileMetallicRoughness.C_Str());
        out.metallicRoughnessTexture = loadTexture(pathMetallicRoughness.C_Str());
    }

    ai_real metallicFactor;
    if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallicFactor))
        out.metallicFactor = glm::clamp(metallicFactor, 0.0f, 1.0f);
    ai_real roughnessFactor;
    if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughnessFactor))
        out.roughnessFactor = glm::clamp(roughnessFactor, 0.0f, 1.0f);

    // normal map

    if(fileNormals.length > 0)
    {
        aiString pathNormals;
        pathNormals.Set(dir);
        pathNormals.Append(fileNormals.C_Str());
        out.normalTexture = loadTexture(pathNormals.C_Str());
    }

    ai_real normalScale;
    if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), normalScale))
        out.normalScale = normalScale;

    // occlusion texture

    if(fileOcclusion == fileMetallicRoughness)
    {
        // some GLTF files combine metallic/roughness and occlusion values into one texture
        // don't load it twice
        out.occlusionTexture = out.metallicRoughnessTexture;
    }
    else if(fileOcclusion.length > 0)
    {
        aiString pathOcclusion;
        pathOcclusion.Set(dir);
        pathOcclusion.Append(fileOcclusion.C_Str());
        out.occlusionTexture = loadTexture(pathOcclusion.C_Str());
    }

    ai_real occlusionStrength;
    if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), occlusionStrength))
        out.occlusionStrength = glm::clamp(occlusionStrength, 0.0f, 1.0f);

    // emissive texture

    if(fileEmissive.length > 0)
    {
        aiString pathEmissive;
        pathEmissive.Set(dir);
        pathEmissive.Append(fileEmissive.C_Str());
        out.emissiveTexture = loadTexture(pathEmissive.C_Str(), true /* sRGB */);
    }

// assimp doesn't define this
#ifndef AI_MATKEY_GLTF_EMISSIVE_FACTOR
#define AI_MATKEY_GLTF_EMISSIVE_FACTOR AI_MATKEY_COLOR_EMISSIVE
#endif

    aiColor3D emissiveFactor;
    if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_EMISSIVE_FACTOR, emissiveFactor))
        out.emissiveFactor = { emissiveFactor.r, emissiveFactor.g, emissiveFactor.b };
    out.emissiveFactor = glm::clamp(out.emissiveFactor, 0.0f, 1.0f);

    return out;
}

Camera Scene::loadCamera(const aiCamera* camera)
{
    float aspect = camera->mAspect == 0.0f ? 16.0f / 9.0f : camera->mAspect;
    glm::vec3 pos(camera->mPosition.x, camera->mPosition.y, camera->mPosition.z);
    glm::vec3 target(camera->mLookAt.x, camera->mLookAt.y, camera->mLookAt.z);
    glm::vec3 up(camera->mUp.x, camera->mUp.y, camera->mUp.z);

    Camera cam;
    cam.lookAt(pos, target, up);
    // convert horizontal half angle (radians) to vertical full angle (degrees)
    cam.fov = glm::degrees(2.0f * glm::atan(glm::tan(camera->mHorizontalFOV) / aspect));
    cam.zNear = camera->mClipPlaneNear;
    cam.zFar = camera->mClipPlaneFar;

    return cam;
}

bgfx::TextureHandle Scene::loadTexture(const char* file, bool sRGB)
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
        const bgfx::Memory* mem = bgfx::makeRef(
            image->m_data,
            image->m_size,
            [](void*, void* data) { bimg::imageFree((bimg::ImageContainer*)data); },
            image);
        BX_FREE(&allocator, data);

        uint64_t textureFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;
        if(sRGB)
            textureFlags |= BGFX_TEXTURE_SRGB;

        if(bgfx::isTextureValid(0, false, image->m_numLayers, (bgfx::TextureFormat::Enum)image->m_format, textureFlags))
        {
            bgfx::TextureHandle tex =
                bgfx::createTexture2D((uint16_t)image->m_width,
                                      (uint16_t)image->m_height,
                                      image->m_numMips > 1,
                                      image->m_numLayers,
                                      (bgfx::TextureFormat::Enum)image->m_format,
                                      textureFlags,
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
