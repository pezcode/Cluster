#include "ForwardRenderer.h"

#include "Scene/Scene.h"
#include <bigg.hpp>
#include <bx/string.h>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

ForwardRenderer::ForwardRenderer(const Scene* scene) :
    Renderer(scene),
    program(BGFX_INVALID_HANDLE)
{
}

ForwardRenderer::~ForwardRenderer()
{
}

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

    bgfx::touch(vDefault); // empty primitive in case nothing follows

    if(!scene->loaded)
        return;

    setViewProjection(vDefault);

    uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;

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
        bgfx::setState(state | materialState);
        bgfx::submit(vDefault, program);
    }
}

void ForwardRenderer::onShutdown()
{
    bgfx::destroy(program);
    program = BGFX_INVALID_HANDLE;
}
