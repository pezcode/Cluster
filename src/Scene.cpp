#include "Scene.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <type_traits>
#include <bx/bx.h>

std::once_flag Scene::onceFlag;
bgfx::VertexDecl Scene::PosNormalTex0Vertex::decl;

Scene::Scene() :
    loaded(false)
{
    static_assert(std::is_same<ai_real, float>::value, "Assimp type must be float");
    Assimp::DefaultLogger::set(&logSource);
}

Scene::~Scene()
{
}

void Scene::init()
{
    std::call_once(onceFlag, []() {
        PosNormalTex0Vertex::init();
    });
}

void Scene::clear()
{
    if(loaded)
    {
        // cleanup
    }
    loaded = false;
}

bool Scene::load(const char* path)
{
    clear();

    Assimp::Importer importer;
    // only take triangles or higher (polygons are triangulated during import)
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

    // TODO? importer.SetProgressHandler

    unsigned int flags = aiProcessPreset_TargetRealtime_Quality;

    // bgfx specific

    // something here is wrong, Renderer::blit culls CW, but quad triangles are in CW
    //flags |= aiProcess_FlipWindingOrder; // flag in bgfx::setState(), default is to cull(!) CW
    flags |= aiProcess_MakeLeftHanded; // we set GLM_FORCE_LEFT_HANDED and use left-handed bx matrix functions
    if(!bgfx::getCaps()->originBottomLeft)
        flags |= aiProcess_FlipUVs;

    // D3D:
    // aiProcess_ConvertToLeftHanded
    // better:
    // use bgfx caps for aiProcess_MakeLeftHanded | aiProcess_FlipUVs |
    // aiProcess_FlipWindingOrder: set winding order to CW (Renderer uses that, and BGFX_STATE_DEFAULT contains CW)

    const aiScene* scene = importer.ReadFile(path, flags);
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

            for(unsigned int i = 0; i < scene->mNumMaterials; i++)
            {
                try
                {
                    materials.push_back(loadMaterial(scene->mMaterials[i]));
                }
                catch(std::exception& e)
                {
                    Log->warn("{}", e.what());
                }
            }

            loaded = true;
        }
        else
        {
            Log->error("Scene is incomplete or invalid");
        }
    }
    else
    {
        Log->error(importer.GetErrorString());
    }

    return loaded;
}

Scene::Mesh Scene::loadMesh(const aiMesh* mesh)
{
    if(mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
        throw std::runtime_error("Mesh has incompatible primitive type");

    constexpr size_t coords = 0;
    bool hasTexture = mesh->mNumUVComponents[coords] == 2 && mesh->mTextureCoords[coords] != nullptr;

    // vertices

    const bgfx::Memory* vMem = bgfx::alloc(mesh->mNumVertices * sizeof(PosNormalTex0Vertex));
    PosNormalTex0Vertex* vertices = (PosNormalTex0Vertex*)vMem->data;

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

    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(vMem, PosNormalTex0Vertex::decl);

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

Scene::Material Scene::loadMaterial(const aiMaterial* material)
{
    // TODO
    throw std::runtime_error("Not implemented");

    //material->
}
