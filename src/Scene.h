#pragma once

#include "Camera.h"
#include "Log/AssimpSource.h"
#include <assimp/mesh.h>
#include <assimp/material.h>
#include <bgfx/bgfx.h>
#include <mutex>

class Scene
{
public:
    Scene();
    ~Scene();

    static void init();

    bool load(const char* path);
    void clear();

    Camera camera;

    bool loaded;

    struct Mesh
    {
        bgfx::VertexBufferHandle vertexBuffer;
        bgfx::IndexBufferHandle indexBuffer;
        unsigned int material; // index into materials vector
    };

    struct Material
    {
        bgfx::TextureHandle albedo;
    };

    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    struct PosNormalTex0Vertex
    {
        float x, y, z;
        float nx, ny, nz;
        float tx, ty, tz;
        float u, v;

        static void init()
        {
            decl.begin()
                .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Normal,    3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Tangent,   3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();
        }
        static bgfx::VertexDecl decl;
    };

private:
    static std::once_flag onceFlag;
    AssimpLogSource logSource;

    Mesh loadMesh(const aiMesh* mesh);
    Material loadMaterial(const aiMaterial* material);
};
