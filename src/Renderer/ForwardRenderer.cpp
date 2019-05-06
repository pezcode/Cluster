#include "ForwardRenderer.h"

#include <bigg.hpp>
#include <bx/string.h>
#include <bx/math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

ForwardRenderer::ForwardRenderer(const Scene* scene) :
    Renderer(scene),
    program(BGFX_INVALID_HANDLE),
    mTime(0.0f)
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

void ForwardRenderer::onReset()
{
    
}

void ForwardRenderer::onRender(float dt)
{
    bgfx::ViewId vDefault = 0;

    mTime += dt;

    glm::mat4 view, proj;
    bx::mtxLookAt(&view[0][0], { 0.0f, 0.0f, -25.0f }, { 0.0f, 0.0f, 0.0f});
    bx::mtxProj(&proj[0][0], scene->camera.fov, float(width) / height, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(vDefault, &view[0][0], &proj[0][0]);

    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030FF, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);

    bgfx::touch(vDefault); // empty primitive in case nothing follows

    if(!scene->loaded)
        return;

    for(size_t i = 0; i < scene->meshes.size(); i++)
    {
        glm::mat4 mtx;
        mtx = glm::translate(mtx, glm::vec3(0.0f, -10.0f, 0.0f));
        mtx = glm::translate(mtx, glm::vec3(5.0f, 0.0f, 20.0f));
        mtx = glm::scale(mtx, glm::vec3(0.1f));
        mtx = glm::rotate(mtx, mTime * 5.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        bgfx::setTransform(&mtx[0][0]);
        bgfx::setVertexBuffer(0, scene->meshes[i].vertexBuffer);
        bgfx::setIndexBuffer(scene->meshes[i].indexBuffer);
        bgfx::setState(BGFX_STATE_DEFAULT);
        bgfx::submit(vDefault, program);
    }
}

void ForwardRenderer::onShutdown()
{
    bgfx::destroy(program);
}
