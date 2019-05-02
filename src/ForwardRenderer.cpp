#include "ForwardRenderer.h"

#include <bigg.hpp>
#include <bx/string.h>
#include <bx/math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

ForwardRenderer::ForwardRenderer(const Scene* scene) :
    Renderer(scene),
    mProgram(BGFX_INVALID_HANDLE),
    mVbh(BGFX_INVALID_HANDLE),
    mIbh(BGFX_INVALID_HANDLE),
    mTime(0.0f)
{
}

ForwardRenderer::~ForwardRenderer()
{
}

void ForwardRenderer::initialize()
{
    Renderer::initialize();

    // TODO do this somewhere else (probably Scene)
    PosColorVertex::init();

    char vsName[32];
    char fsName[32];

    const char* dir = shaderDir();

    bx::strCopy(vsName, BX_COUNTOF(vsName), dir);
    bx::strCat(vsName, BX_COUNTOF(vsName), "vs_cubes.bin");

    bx::strCopy(fsName, BX_COUNTOF(fsName), dir);
    bx::strCat(fsName, BX_COUNTOF(fsName), "fs_cubes.bin");

    mProgram = bigg::loadProgram(vsName, fsName);
    mVbh = bgfx::createVertexBuffer(bgfx::makeRef(scene->s_cubeVertices, sizeof(scene->s_cubeVertices)),
                                    PosColorVertex::ms_decl);
    mIbh = bgfx::createIndexBuffer(bgfx::makeRef(scene->s_cubeTriList, sizeof(scene->s_cubeTriList)));
}

void ForwardRenderer::reset(uint16_t width, uint16_t height)
{
    Renderer::reset(width, height);
}

void ForwardRenderer::render(float dt)
{
    bgfx::ViewId vDefault = 0;
    bgfx::ViewId vBlitToScreen = 199; // imgui in bigg is 200

    mTime += dt;
    glm::mat4 view =
        glm::lookAt(glm::vec3(0.0f, 0.0f, -25.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = bigg::perspective(glm::radians(scene->camera.fov), float(width) / height, 0.1f, 100.0f);
    bgfx::setViewTransform(vDefault, &view[0][0], &proj[0][0]);

    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030FF, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);

    for(uint32_t yy = 0; yy < 11; ++yy)
    {
        for(uint32_t xx = 0; xx < 11; ++xx)
        {
            glm::mat4 mtx;
            mtx = glm::translate(mtx, glm::vec3(15.0f - float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 0.0f));
            mtx *= glm::yawPitchRoll(mTime + xx * 0.21f, mTime + yy * 0.37f, 0.0f);
            bgfx::setTransform(&mtx[0][0]);
            bgfx::setVertexBuffer(0, mVbh);
            bgfx::setIndexBuffer(mIbh);
            bgfx::setState(BGFX_STATE_DEFAULT);
            bgfx::submit(vDefault, mProgram);
        }
    }

    blitToScreen(vBlitToScreen);
}

void ForwardRenderer::shutdown()
{
    Renderer::shutdown();

    bgfx::destroy(mProgram);
    bgfx::destroy(mIbh);
    bgfx::destroy(mVbh);
}
