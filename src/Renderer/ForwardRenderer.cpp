#include "ForwardRenderer.h"

#include "Scene/Scene.h"
#include <bigg.hpp>
#include <bx/string.h>
#include <bx/math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>

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
    if(Renderer::supported())
    {
        return true;
    }
    return false;
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

    // view matrix
    glm::mat4 view = scene->camera.matrix();
    // scale down to camera far plane
    float size = glm::compMax(glm::abs(scene->maxBounds - scene->minBounds));
    view = glm::scale(view, glm::vec3(1.0f / size * scene->camera.zFar));

    // projection matrix
    glm::mat4 proj;
    bx::mtxProj(&proj[0][0], scene->camera.fov, float(width) / height,
                scene->camera.zNear, scene->camera.zFar, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(vDefault, &view[0][0], &proj[0][0]);

    for(const Mesh& mesh : scene->meshes)
    {
        bgfx::setVertexBuffer(0, mesh.vertexBuffer);
        bgfx::setIndexBuffer(mesh.indexBuffer);
        const Material& mat = scene->materials[mesh.material];
        uint64_t materialState = pbr.bindMaterial(mat);
        uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;
        bgfx::setState(state | materialState);
        bgfx::submit(vDefault, program);
    }
}

void ForwardRenderer::onShutdown()
{
    bgfx::destroy(program);
}
