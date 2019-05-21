#include "ClusteredRenderer.h"

#include "Scene/Scene.h"
#include <bigg.hpp>
#include <bx/string.h>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

ClusteredRenderer::ClusteredRenderer(const Scene* scene) :
    Renderer(scene),
    lightingProgram(BGFX_INVALID_HANDLE),
    lightCullingComputeProgram(BGFX_INVALID_HANDLE)
{
}

ClusteredRenderer::~ClusteredRenderer()
{
}

bool ClusteredRenderer::supported()
{
    if(Renderer::supported())
    {
        return true;
    }
    return false;
}

void ClusteredRenderer::onInitialize()
{
    char vsName[128], fsName[128], csName[128];
    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_clustered.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_clustered.bin");
    bx::snprintf(csName, BX_COUNTOF(csName), "%s%s", shaderDir(), "cs_clustered_lightculling.bin");
    lightingProgram = bigg::loadProgram(vsName, fsName);
    lightCullingComputeProgram = bgfx::createProgram(bigg::loadShader(csName), true);
}

void ClusteredRenderer::onRender(float dt)
{
    enum : bgfx::ViewId
    {
        vLightCulling = 0,
        vLighting
    };

    bgfx::setViewName(vLightCulling, "Clustered light culling pass (compute)");
    bgfx::touch(vLightCulling);

    bgfx::setViewName(vLighting, "Clustered lighting pass");
    bgfx::setViewClear(vLighting, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0);
    bgfx::setViewRect(vLighting, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vLighting, frameBuffer);
    bgfx::touch(vLighting);

    if(!scene->loaded)
        return;

    bgfx::dispatch(vLightCulling, lightCullingComputeProgram, CLUSTER_WIDTH, CLUSTER_HEIGHT);

    setViewProjection(vLighting);

    uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;

    for(const Mesh& mesh : scene->meshes)
    {
        glm::mat4 model = glm::mat4();
        bgfx::setTransform(glm::value_ptr(model));
        setNormalMatrix(model);
        bgfx::setVertexBuffer(0, mesh.vertexBuffer);
        bgfx::setIndexBuffer(mesh.indexBuffer);
        const Material& mat = scene->materials[mesh.material];
        uint64_t materialState = pbr.bindMaterial(mat);
        uint64_t lightState = lights.bindLights(scene);
        bgfx::setState(state | materialState | lightState);
        bgfx::submit(vLighting, lightingProgram);
    }
}

void ClusteredRenderer::onShutdown()
{
    bgfx::destroy(lightingProgram);
    bgfx::destroy(lightCullingComputeProgram);
    lightingProgram = lightCullingComputeProgram = BGFX_INVALID_HANDLE;
}
