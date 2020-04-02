#pragma once

#include <bgfx/bgfx.h>

class Scene;

class ClusterShader
{
public:
    ClusterShader();

    void initialize();
    void shutdown();

    void setUniforms(const Scene* scene, uint16_t screenWidth, uint16_t screenHeight) const;
    void bindBuffers(bool lightingPass = true) const;

    static constexpr uint32_t CLUSTERS_X = 16;
    static constexpr uint32_t CLUSTERS_Y = 8;
    static constexpr uint32_t CLUSTERS_Z = 24;

    // limit number of threads (D3D only allows up to 1024, there might also be shared memory limitations)
    // shader will be run by 6 work groups
    static constexpr uint32_t CLUSTERS_X_THREADS = 16;
    static constexpr uint32_t CLUSTERS_Y_THREADS = 8;
    static constexpr uint32_t CLUSTERS_Z_THREADS = 4;

    static constexpr uint32_t CLUSTER_COUNT = CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z;

    static constexpr uint32_t MAX_LIGHTS_PER_CLUSTER = 100;

private:
    struct ClusterVertex
    {
        // w is padding
        float minBounds[4];
        float maxBounds[4];

        static void init()
        {
            layout.begin()
                .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
                .end();
        }
        static bgfx::VertexLayout layout;
    };

    bgfx::UniformHandle clusterSizesVecUniform;
    bgfx::UniformHandle zNearFarVecUniform;

    // dynamic buffers can be created empty
    bgfx::DynamicVertexBufferHandle clustersBuffer;
    bgfx::DynamicIndexBufferHandle lightIndicesBuffer;
    bgfx::DynamicIndexBufferHandle lightGridBuffer;
    bgfx::DynamicIndexBufferHandle atomicIndexBuffer;

    uint32_t lightCount;
};
