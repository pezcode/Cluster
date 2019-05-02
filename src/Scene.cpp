#include "Scene.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

bgfx::VertexDecl PosColorVertex::ms_decl;

Scene::Scene() :
    s_cubeVertices{
        { -1.0f, 1.0f, 1.0f, 0xff000000 },   { 1.0f, 1.0f, 1.0f, 0xff0000ff },   { -1.0f, -1.0f, 1.0f, 0xff00ff00 },
        { 1.0f, -1.0f, 1.0f, 0xff00ffff },   { -1.0f, 1.0f, -1.0f, 0xffff0000 }, { 1.0f, 1.0f, -1.0f, 0xffff00ff },
        { -1.0f, -1.0f, -1.0f, 0xffffff00 }, { 1.0f, -1.0f, -1.0f, 0xffffffff }
    },
    s_cubeTriList {
        2, 1, 0, 2, 3, 1, 5, 6, 4, 7, 6, 5, 4, 2, 0, 6, 2, 4,
        3, 5, 1, 3, 7, 5, 1, 4, 0, 1, 5, 4, 6, 3, 2, 7, 3, 6
    }
{
    Assimp::DefaultLogger::set(&logSource);
}

Scene::~Scene()
{
}

void Scene::clear()
{
}

void Scene::load(const char* path)
{
    clear();

    // TODO
    // importer.SetProgressHandler

    unsigned int flags = aiProcessPreset_TargetRealtime_Quality;
    // D3D:
    // aiProcess_ConvertToLeftHanded
    // better:
    // use bgfx caps for aiProcess_MakeLeftHanded | aiProcess_FlipUVs | 
    // aiProcess_FlipWindingOrder: set winding order to CW (Renderer uses that, and BGFX_STATE_DEFAULT contains CW)

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, flags);
    if(scene)
    {
        if(!(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && (scene->mFlags & AI_SCENE_FLAGS_VALIDATED))
        {
            for(unsigned int i = 0; i < scene->mNumMeshes; i++)
            {
                const aiMesh* mesh = scene->mMeshes[i];
                loadMesh(mesh);
            }

            for(unsigned int i = 0; i < scene->mNumMaterials; i++)
            {
                const aiMaterial* material = scene->mMaterials[i];
                loadMaterial(material);
            }
        }
        else
        {
            Log->error("Scene is incomplete or invalid");
        }
    }
    else
    {
        // TODO
        //log(importer.GetErrorString());
    }
}

void Scene::loadMesh(const aiMesh* mesh)
{
}

void Scene::loadMaterial(const aiMaterial* material)
{
}
