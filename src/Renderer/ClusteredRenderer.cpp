#include "ClusteredRenderer.h"

ClusteredRenderer::ClusteredRenderer(const Scene* scene) :
    Renderer(scene)
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
    
}

void ClusteredRenderer::onReset()
{
    
}

void ClusteredRenderer::onRender(float dt)
{
    bgfx::ViewId vDefault = 0;
    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030FF, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);
    bgfx::touch(vDefault);
}

void ClusteredRenderer::onShutdown()
{
    
}
