#pragma once

#include "Camera.h"
#include "Log/AssimpSource.h"
#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include <mutex>

struct aiMesh;
struct aiMaterial;
struct aiCamera;

class Scene
{
public:
    Scene();
    ~Scene();

    static void init();

    bool load(const char* file);
    void clear();

    Camera camera;

    bool loaded;

    struct Mesh
    {
        bgfx::VertexBufferHandle vertexBuffer = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle indexBuffer = BGFX_INVALID_HANDLE;
        unsigned int material = 0; // index into materials vector
    };

    struct Material
    {
        bgfx::TextureHandle baseColor = BGFX_INVALID_HANDLE;
        bgfx::TextureHandle metallicRoughness = BGFX_INVALID_HANDLE;
    };

    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    struct PosNormalTangentTex0Vertex
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
    Material loadMaterial(const aiMaterial* material, const char* dir);
    Camera loadCamera(const aiCamera* camera);

    static bx::DefaultAllocator allocator;
    static bgfx::TextureHandle loadTexture(const char* file);
};
