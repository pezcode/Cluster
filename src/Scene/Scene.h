#pragma once

#include "Scene/Camera.h"
#include "Scene/Mesh.h"
#include "Scene/Material.h"
#include "Scene/Light.h"
#include "Scene/LightList.h"
#include "Log/AssimpSource.h"
#include <glm/matrix.hpp>
#include <bgfx/bgfx.h>
#include <bx/allocator.h>

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
    glm::vec3 skyColor;
    PointLightList pointLights;

private:

    static bx::DefaultAllocator allocator;
    AssimpLogSource logSource;

    Mesh loadMesh(const aiMesh* mesh); // not static because it changes minBounds and maxBounds
    static Material loadMaterial(const aiMaterial* material, const char* dir);
    static Camera loadCamera(const aiCamera* camera);

    static bgfx::TextureHandle loadTexture(const char* file);
};
