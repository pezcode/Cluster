#include "ForwardRenderer.h"

#include "Scene/Scene.h"
#include <bigg.hpp>
#include <bx/string.h>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

ForwardRenderer::ForwardRenderer(const Scene* scene) : Renderer(scene) { }

bool ForwardRenderer::supported()
{
    return Renderer::supported();
}

void ForwardRenderer::onInitialize()
{
    char vsName[128], fsName[128];
    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_forward.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_forward.bin");
    program = bigg::loadProgram(vsName, fsName);
}

void ForwardRenderer::onRender(float dt)
{
    bgfx::ViewId vDefault = 0;

    bgfx::setViewName(vDefault, "Forward render pass");
    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);

    // empty primitive in case nothing follows
    // this makes sure the clear happens
    bgfx::touch(vDefault);

    if(!scene->loaded)
        return;

    setViewProjection(vDefault);

    uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;

    pbr.bindAlbedoLUT();
    lights.bindLights(scene);

    for(const Mesh& mesh : scene->meshes)
    {
        glm::mat4 model = glm::identity<glm::mat4>();
        bgfx::setTransform(glm::value_ptr(model));
        setNormalMatrix(model);
        bgfx::setVertexBuffer(0, mesh.vertexBuffer);
        bgfx::setIndexBuffer(mesh.indexBuffer);
        const Material& mat = scene->materials[mesh.material];
        uint64_t materialState = pbr.bindMaterial(mat);
        bgfx::setState(state | materialState);
        bgfx::submit(vDefault, program, 0, ~BGFX_DISCARD_BINDINGS);
    }

    bgfx::discard(BGFX_DISCARD_ALL);
}

void ForwardRenderer::onShutdown()
{
    bgfx::destroy(program);
    program = BGFX_INVALID_HANDLE;
}
