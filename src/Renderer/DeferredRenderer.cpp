#include "DeferredRenderer.h"

DeferredRenderer::DeferredRenderer(const Scene* scene) :
    Renderer(scene),
    bufferList{ { BGFX_INVALID_HANDLE, "Albedo"   },
                { BGFX_INVALID_HANDLE, "Normal"   },
                { BGFX_INVALID_HANDLE, "Specular" },
                { BGFX_INVALID_HANDLE, nullptr    } }
{
    buffers = bufferList;
}

DeferredRenderer::~DeferredRenderer()
{

}

bool DeferredRenderer::supported()
{
    if(Renderer::supported())
    {
        return true;
    }
    return false;
}

void DeferredRenderer::onInitialize()
{

}

void DeferredRenderer::onReset()
{

}

void DeferredRenderer::onRender(float dt)
{
    bgfx::ViewId vDefault = 0;
    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);
    bgfx::touch(vDefault);
}

void DeferredRenderer::onShutdown()
{

}
