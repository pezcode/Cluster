#pragma once

#include "Scene/Camera.h"
#include "Scene/Light.h"
#include "Scene/Mesh.h"
#include "Scene/Material.h"
#include "Log/AssimpSource.h"
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include <vector>
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
