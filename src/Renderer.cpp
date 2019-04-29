#include "Renderer.h"

void Renderer::initialize()
{
    // TODO create framebuffer
    // this uses the default framebuffer for now
    Renderer::frameBuffer = BGFX_INVALID_HANDLE;
}

void Renderer::reset(uint16_t width, uint16_t height)
{
    this->width = width;
    this->height = height;
    // TODO create new framebuffer
}

void Renderer::shutdown()
{
    // TODO free framebuffer
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
