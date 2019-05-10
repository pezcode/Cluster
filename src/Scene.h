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

    // load meshes, materials, camera from .gltf file
    bool load(const char* file);
    void clear();

    struct Mesh
    {
        bgfx::VertexBufferHandle vertexBuffer = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle indexBuffer = BGFX_INVALID_HANDLE;
        unsigned int material = 0; // index into materials vector
    };

    // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#metallic-roughness-material
    struct Material
    {
        bool doubleSided = false;
        glm::vec4 baseColor = { 1.0f, 0.0f, 1.0f, 1.0f }; // normalized RGBA
        float metallic = 0.0f;
        float roughness = 0.5f;
        bgfx::TextureHandle baseColorTexture = BGFX_INVALID_HANDLE;         // F0 for non-metals
        bgfx::TextureHandle metallicRoughnessTexture = BGFX_INVALID_HANDLE; // blue = metallic, green = roughness
        bgfx::TextureHandle normalTexture = BGFX_INVALID_HANDLE;
    };

    struct PointLight
    {
        glm::vec3 position;
        glm::vec4 color;
    };

    struct DirectionalLight
    {
        glm::vec3 direction;
        glm::vec4 color;
    };

    struct SpotLight : PointLight
    {
        glm::vec3 direction;
        float angle; // full angle in degrees
    };

    bool loaded;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
    Camera camera;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    // sky color and lights are not populated by load
    glm::vec4 skyColor;
    std::vector<PointLight> pointLights;
    std::vector<PointLight> spotLights;
    DirectionalLight ambientLight;

private:
    struct PosNormalTangentTex0Vertex
    {
        float x, y, z;
        float nx, ny, nz;
        float tx, ty, tz;
        float u, v;

        static void init()
        {
            decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();
        }
        static bgfx::VertexDecl decl;
    };

    static std::once_flag onceFlag;
    AssimpLogSource logSource;

    Mesh loadMesh(const aiMesh* mesh);
    Material loadMaterial(const aiMaterial* material, const char* dir);
    Camera loadCamera(const aiCamera* camera);

    static bx::DefaultAllocator allocator;
    static bgfx::TextureHandle loadTexture(const char* file);
};
