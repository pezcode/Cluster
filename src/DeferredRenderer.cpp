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

void DeferredRenderer::initialize()
{
    Renderer::initialize();
}

void DeferredRenderer::reset(uint16_t width, uint16_t height)
{
    Renderer::reset(width, height);
}

void DeferredRenderer::render(float dt)
{
    
}

void DeferredRenderer::shutdown()
{
    Renderer::shutdown();
}
