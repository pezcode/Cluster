#include "ClusteredRenderer.h"

#include "Scene/Scene.h"
#include <bigg.hpp>
#include <bx/string.h>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

ClusteredRenderer::ClusteredRenderer(const Scene* scene) :
    Renderer(scene),
    clusterBuildingComputeProgram(BGFX_INVALID_HANDLE),
    lightCullingComputeProgram(BGFX_INVALID_HANDLE),
    lightingProgram(BGFX_INVALID_HANDLE),
    debugVisProgram(BGFX_INVALID_HANDLE)
{
}

ClusteredRenderer::~ClusteredRenderer()
{
}

bool ClusteredRenderer::supported()
{
    const bgfx::Caps* caps = bgfx::getCaps();
    return Renderer::supported() &&
        (caps->supported & BGFX_CAPS_COMPUTE) != 0 &&
        (caps->supported & BGFX_CAPS_INDEX32) != 0;
}

void ClusteredRenderer::onInitialize()
{
    // OpenGL backend: uniforms must be created before loading shaders
    clusters.initialize();

    char csClusterBuildingName[128], csLightCullingName[128], vsName[128], fsName[128], fsDebugVisName[128];
    bx::snprintf(csClusterBuildingName, BX_COUNTOF(csClusterBuildingName), "%s%s", shaderDir(), "cs_clustered_clusterbuilding.bin");
    bx::snprintf(csLightCullingName, BX_COUNTOF(csLightCullingName), "%s%s", shaderDir(), "cs_clustered_lightculling.bin");
    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_clustered.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_clustered.bin");
    bx::snprintf(fsDebugVisName, BX_COUNTOF(fsDebugVisName), "%s%s", shaderDir(), "fs_clustered_debug_vis.bin");

    clusterBuildingComputeProgram = bgfx::createProgram(bigg::loadShader(csClusterBuildingName), true);
    lightCullingComputeProgram = bgfx::createProgram(bigg::loadShader(csLightCullingName), true);
    lightingProgram = bigg::loadProgram(vsName, fsName);
    debugVisProgram = bigg::loadProgram(vsName, fsDebugVisName);
}

void ClusteredRenderer::onRender(float dt)
{
    enum : bgfx::ViewId
    {
        vClusterBuilding = 0,
        vLightCulling,
        vLighting
    };

    bgfx::touch(vClusterBuilding);
    bgfx::touch(vLightCulling);

    bgfx::setViewClear(vLighting, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0);
    bgfx::setViewRect(vLighting, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vLighting, frameBuffer);
    bgfx::touch(vLighting);

    if(!scene->loaded)
        return;

    // cluster building needs u_invProj to transform screen coordinates to eye space
    setViewProjection(vClusterBuilding);
    // light culling needs u_view to transform lights to eye space
    setViewProjection(vLightCulling);

    clusters.setUniforms(scene, width, height);

    {
        bgfx::setViewName(vClusterBuilding, "Cluster building pass (compute)");

        clusters.bindBuffers(false); // write access, all buffers

        bgfx::dispatch(vClusterBuilding,
                       clusterBuildingComputeProgram,
                       ClusterShader::CLUSTERS_X,
                       ClusterShader::CLUSTERS_Y,
                       ClusterShader::CLUSTERS_Z);
    }

    {
        bgfx::setViewName(vLightCulling, "Clustered light culling pass (compute)");

        lights.bindLights(scene);
        clusters.bindBuffers(false); // write access, all buffers

        bgfx::dispatch(vLightCulling,
                       lightCullingComputeProgram,
                       1,
                       1,
                       ClusterShader::CLUSTERS_Z / ClusterShader::CLUSTERS_Z_THREADS);
    }

    bgfx::setViewName(vLighting, "Clustered lighting pass");

    setViewProjection(vLighting);

    uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;

    bool debugVis = variables["DEBUG_VIS"] == "true";
    bgfx::ProgramHandle program = debugVis ? debugVisProgram : lightingProgram;

    for(const Mesh& mesh : scene->meshes)
    {
        glm::mat4 model = glm::identity<glm::mat4>();
        bgfx::setTransform(glm::value_ptr(model));
        setNormalMatrix(model);
        bgfx::setVertexBuffer(0, mesh.vertexBuffer);
        bgfx::setIndexBuffer(mesh.indexBuffer);
        const Material& mat = scene->materials[mesh.material];
        uint64_t materialState = pbr.bindMaterial(mat);
        lights.bindLights(scene);
        clusters.bindBuffers();
        bgfx::setState(state | materialState);
        bgfx::submit(vLighting, program);
    }
}

void ClusteredRenderer::onShutdown()
{
    clusters.shutdown();

    bgfx::destroy(clusterBuildingComputeProgram);
    bgfx::destroy(lightCullingComputeProgram);
    bgfx::destroy(lightingProgram);
    bgfx::destroy(debugVisProgram);

    clusterBuildingComputeProgram = lightCullingComputeProgram = lightingProgram = debugVisProgram = BGFX_INVALID_HANDLE;
}
