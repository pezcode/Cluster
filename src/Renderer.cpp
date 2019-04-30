#include "Renderer.h"

#include <bx/macros.h>

void Renderer::initialize()
{
    frameBuffer = bgfx::createFrameBuffer(bgfx::BackbufferRatio::Equal, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_READ_BACK);
}

void Renderer::reset(uint16_t width, uint16_t height)
{
    if(!bgfx::isValid(frameBuffer) || width != this->width || height != this->height)
    {
        if(bgfx::isValid(frameBuffer))
        {
            //bgfx::destroy(frameBuffer);
        }

        //frameBuffer =
        //    bgfx::createFrameBuffer(bgfx::BackbufferRatio::Equal, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_READ_BACK);

        //bgfx::TextureHandle textures[2];
        //uint32_t flags = 0;

        //textures[0] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, flags);

        //frameBuffer = bgfx::createFrameBuffer(BX_COUNTOF(textures), textures, true);

        //frameBuffer = bgfx::createFrameBuffer(bgfx::BackbufferRatio::Equal, bgfx::TextureFormat::BGRA8);
    }
    this->width = width;
    this->height = height;
}

void Renderer::shutdown()
{
    bgfx::destroy(frameBuffer);
}

const char* Renderer::shaderDir()
{
    const char* path = "???";

    switch(bgfx::getRendererType())
    {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D9:
            path = "shaders/dx9/";
            break;
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            path = "shaders/dx11/";
            break;
        case bgfx::RendererType::Gnm:
            break;
        case bgfx::RendererType::Metal:
            path = "shaders/metal/";
            break;
        case bgfx::RendererType::OpenGL:
            path = "shaders/glsl/";
            break;
        case bgfx::RendererType::OpenGLES:
            path = "shaders/essl/";
            break;
        case bgfx::RendererType::Vulkan:
            break;
        case bgfx::RendererType::Count:
            break;
    }

    return path;
}
