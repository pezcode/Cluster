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

    // D3D compute shaders only allow up to 1024 threads
    // shader will be invoked 3 times
    static constexpr uint32_t CLUSTERS_Z_THREADS = 8;

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
            decl.begin()
                .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
                .end();
        }
        static bgfx::VertexDecl decl;
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
