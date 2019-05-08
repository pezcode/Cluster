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

std::once_flag Scene::onceFlag;
bgfx::VertexDecl Scene::PosNormalTangentTex0Vertex::decl;
bx::DefaultAllocator Scene::allocator;

Scene::Scene() :
    loaded(false)
{
    Assimp::DefaultLogger::set(&logSource);
}

Scene::~Scene()
{
}

void Scene::init()
{
    std::call_once(onceFlag, []() { PosNormalTangentTex0Vertex::init();
    });
}

void Scene::clear()
{
    if(loaded)
    {
        camera = Camera();
        for(const Mesh& mesh : meshes)
        {
            bgfx::destroy(mesh.vertexBuffer);
            bgfx::destroy(mesh.indexBuffer);
        }
        meshes.clear();
        for(const Material& mat : materials)
        {
            bgfx::destroy(mat.baseColor);
            bgfx::destroy(mat.metallicRoughness);
        }
        materials.clear();
    }
    loaded = false;
}

bool Scene::load(const char* file)
{
    clear();

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
        aiProcess_FlipUVs;               // bimg loads textures with flipped Y

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
                    // TODO generate error texture (8x8 pink)
                    materials.push_back(Material());
                    Log->warn("{}", e.what());
                }
            }

            if(scene->HasCameras())
            {
                camera = loadCamera(scene->mCameras[0]);
            }
            else
                Log->info("No camera, using default");

            loaded = true;
        }
        else
            Log->error("Scene is incomplete or invalid");
    }
    else
        Log->error(importer.GetErrorString());

    return loaded;
}

Scene::Mesh Scene::loadMesh(const aiMesh* mesh)
{
    if(mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
        throw std::runtime_error("Mesh has incompatible primitive type");

    constexpr size_t coords = 0;
    bool hasTexture = mesh->mNumUVComponents[coords] == 2 && mesh->mTextureCoords[coords] != nullptr;

    // vertices

    const bgfx::Memory* vMem = bgfx::alloc(mesh->mNumVertices * sizeof(PosNormalTangentTex0Vertex));
    PosNormalTangentTex0Vertex* vertices = (PosNormalTangentTex0Vertex*)vMem->data;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        aiVector3D pos = mesh->mVertices[i];
        vertices[i].x = pos.x;
        vertices[i].y = pos.y;
        vertices[i].z = pos.z;

        aiVector3D nrm = mesh->mNormals[i];
        vertices[i].nx = nrm.x;
        vertices[i].ny = nrm.y;
        vertices[i].nz = nrm.z;

        aiVector3D tan = mesh->mTangents[i];
        vertices[i].tx = tan.x;
        vertices[i].ty = tan.y;
        vertices[i].tz = tan.z;

        if(hasTexture)
        {
            aiVector3D uv = mesh->mTextureCoords[coords][i];
            vertices[i].u = uv.x;
            vertices[i].v = uv.y;
        }
    }

    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(vMem, PosNormalTangentTex0Vertex::decl);

    // indices

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

    return { vbh, ibh, mesh->mMaterialIndex };
}

Scene::Material Scene::loadMaterial(const aiMaterial* material, const char* dir)
{
    aiString fileBaseColor, fileMetallicRoughness;
    material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &fileBaseColor);
    material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &fileMetallicRoughness);
    if(fileBaseColor.length == 0)
        throw std::runtime_error("No PBR base color texture");
    // disabled for duck testing
    //if(fileMetallicRoughness.length == 0)
    //    throw std::runtime_error("No PBR metallic/roughness texture");

    aiString pathBaseColor, pathMetallicRoughness;
    pathBaseColor.Set(dir);
    pathBaseColor.Append(fileBaseColor.C_Str());
    pathMetallicRoughness.Set(dir);
    pathMetallicRoughness.Append(fileMetallicRoughness.C_Str());

    return
    {
        loadTexture(pathBaseColor.C_Str()),
        BGFX_INVALID_HANDLE//loadTexture(pathMetallicRoughness.C_Str())
    };
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
                                                            BGFX_TEXTURE_NONE,
                                                            mem);
            bgfx::setName(tex, file);
            return tex;
        }
        else
            throw std::runtime_error("Unsupported image format");
    }

    BX_FREE(&allocator, data);
    throw std::runtime_error(err.getMessage().getPtr());
}
