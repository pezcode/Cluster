#include "Renderer.h"

#include "Scene/Scene.h"
#include <bigg.hpp>
#include <bx/macros.h>
#include <bx/string.h>
#include <glm/common.hpp>

bgfx::VertexDecl Renderer::PosTexCoord0Vertex::decl;

Renderer::Renderer(const Scene* scene) :
    scene(scene),
    width(0),
    height(0),
    time(0.0f),
    frameBuffer(BGFX_INVALID_HANDLE),
    blitProgram(BGFX_INVALID_HANDLE),
    blitSampler(BGFX_INVALID_HANDLE)
{
}

Renderer::~Renderer()
{
}

void Renderer::initialize()
{
    PosTexCoord0Vertex::init();

    blitSampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    exposureVecUniform = bgfx::createUniform("u_exposureVec", bgfx::UniformType::Vec4);

    // TODO use triangle covering screen (less fragment overdraw)
    float BOTTOM = -1.0f, TOP = 1.0f, LEFT = -1.0f, RIGHT = 1.0f;
    PosTexCoord0Vertex vertices[6] = {
        { BOTTOM, LEFT }, { TOP, LEFT },  { TOP, RIGHT },
        { BOTTOM, LEFT }, { TOP, RIGHT }, { BOTTOM, RIGHT }
    };
    bool flipV = !bgfx::getCaps()->originBottomLeft;
    for(PosTexCoord0Vertex& v : vertices)
    {
        v.z = 0.0f;
        v.u = (v.x + 1.0f) * 0.5f;
        v.v = (v.y + 1.0f) * 0.5f;
        if(flipV)
            v.v = 1.0f - v.v;
    }
    quadVB = bgfx::createVertexBuffer(bgfx::copy(&vertices, sizeof(vertices)), PosTexCoord0Vertex::decl);

    char vsName[128], fsName[128];
    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_tonemap.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_tonemap.bin");
    blitProgram = bigg::loadProgram(vsName, fsName);

    pbr.initialize();

    onInitialize();
}

void Renderer::reset(uint16_t width, uint16_t height)
{
    if(!bgfx::isValid(frameBuffer) || width != this->width || height != this->height)
    {
        bgfx::FrameBufferHandle newFrameBuffer = createFrameBuffer(width, height, true, true);
        if(bgfx::isValid(frameBuffer))
        {
            // this seems to cause problems when resizing windows too much
            // if you grab and resize the window frame for a few seconds
            // it seems to create buffers but not destroy them
            // TODO try non-threaded
            // or use FrameBuffer that resizes automatically
            // but how to do that for G-Buffer?
            // don't allow resizing? easiest method
            bgfx::destroy(frameBuffer);
        }
        frameBuffer = newFrameBuffer;
    }
    this->width = width;
    this->height = height;

    onReset();
}

void Renderer::render(float dt)
{
    time += dt;

    if(scene->loaded)
    {
        // glm has packUnorm4x8 but it depends on endianness
        glm::u8vec4 result = glm::round(glm::clamp(scene->skyColor, 0.0f, 1.0f) * 255.0f);
        clearColor = (result[0] << 24) | (result[1] << 16) | (result[2] << 8) | result[0];
    }
    else
        clearColor = 0x303030FF;

    onRender(dt);
    blitToScreen(199);
}

void Renderer::shutdown()
{
    onShutdown();

    pbr.shutdown();
    bgfx::destroy(blitProgram);
    bgfx::destroy(blitSampler);
    bgfx::destroy(exposureVecUniform);
    bgfx::destroy(quadVB);
    if(bgfx::isValid(frameBuffer))
        bgfx::destroy(frameBuffer);
}

bool Renderer::supported()
{
    const bgfx::Caps* caps = bgfx::getCaps();
    return (caps->formats[bgfx::TextureFormat::RGBA16F] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER_MSAA) != 0;
}

void Renderer::blitToScreen(bgfx::ViewId view)
{
    bgfx::setViewClear(view, BGFX_CLEAR_NONE);
    bgfx::setViewRect(view, 0, 0, width, height);
    bgfx::setViewFrameBuffer(view, BGFX_INVALID_HANDLE);
    bgfx::setState(BGFX_STATE_WRITE_RGB);
    bgfx::TextureHandle frameBufferTexture = bgfx::getTexture(frameBuffer);
    bgfx::setTexture(0, blitSampler, frameBufferTexture);
    float exposure[4] = { scene->loaded ? scene->camera.exposure : 1.0f, 0.0f, 0.0f, 0.0f };
    bgfx::setUniform(exposureVecUniform, &exposure[0]);
    bgfx::setVertexBuffer(0, quadVB);
    bgfx::submit(view, blitProgram);
}

bgfx::FrameBufferHandle Renderer::createFrameBuffer(uint16_t width, uint16_t height, bool hdr, bool depth)
{
    bgfx::TextureHandle textures[2];
    uint8_t attachments = 0;

    // BGFX_TEXTURE_READ_BACK is not supported for render targets?
    // TODO try blitting for screenshots (new texture with BGFX_TEXTURE_BLIT_DST and BGFX_TEXTURE_READ_BACK)

    uint64_t flags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
    bgfx::TextureFormat::Enum format = hdr
                                       ? bgfx::TextureFormat::RGBA16F
                                       : bgfx::TextureFormat::BGRA8;
    if(bgfx::isTextureValid(0, false, 1, format, flags))
    {
        textures[attachments++] = bgfx::createTexture2D(width, height, false, 1, format, flags);
    }
    // TODO error out

    if(depth)
    {
        flags = BGFX_TEXTURE_RT_WRITE_ONLY;
        bgfx::TextureFormat::Enum depthFormat = bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D16, flags)
                                                ? bgfx::TextureFormat::D16
                                                : bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, flags)
                                                  ? bgfx::TextureFormat::D24S8
                                                  : bgfx::TextureFormat::D32;
        textures[attachments++] = bgfx::createTexture2D(width, height, false, 1, depthFormat, flags);
    }

    bgfx::FrameBufferHandle fb = bgfx::createFrameBuffer(attachments, textures, true);
    
    if(!bgfx::isValid(fb))
        Log->warn("Failed to create framebuffer");

    return fb;
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
