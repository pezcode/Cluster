#include "Renderer.h"

#include <bigg.hpp>
#include <bx/macros.h>
#include <bx/string.h>

bgfx::VertexDecl PosVertex::ms_decl;

Renderer::Renderer(const Scene* scene) :
    scene(scene),
    width(0),
    height(0),
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
    PosVertex::init();

    char vsName[32];
    char fsName[32];

    const char* dir = shaderDir();

    bx::strCopy(vsName, BX_COUNTOF(vsName), dir);
    bx::strCat(vsName, BX_COUNTOF(vsName), "vs_screen_quad.bin");

    bx::strCopy(fsName, BX_COUNTOF(fsName), dir);
    bx::strCat(fsName, BX_COUNTOF(fsName), "fs_screen_quad.bin");

    blitProgram = bigg::loadProgram(vsName, fsName);

    blitSampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
}

void Renderer::reset(uint16_t width, uint16_t height)
{
    if(!bgfx::isValid(frameBuffer) || width != this->width || height != this->height)
    {
        if(bgfx::isValid(frameBuffer))
        {
            bgfx::destroy(frameBuffer);
        }

        bgfx::TextureHandle textures[2];
        uint64_t flags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

        // BGFX_TEXTURE_READ_BACK is not supported for render targets?
        // TODO try blitting for screenshots (new texture with BGFX_TEXTURE_BLIT_DST and BGFX_TEXTURE_READ_BACK)
        if(bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::BGRA8, flags))
        {
            textures[0] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, flags);
        }
        // TODO error out

        flags = BGFX_TEXTURE_RT_WRITE_ONLY;

        bgfx::TextureFormat::Enum depthFormat =
            bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D16, flags)
                ? bgfx::TextureFormat::D16
                : bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D24S8, flags) ? bgfx::TextureFormat::D24S8
                                                                                       : bgfx::TextureFormat::D32;

        textures[1] = bgfx::createTexture2D(width, height, false, 1, depthFormat, flags);

        frameBuffer = bgfx::createFrameBuffer(BX_COUNTOF(textures), textures, true);

        // this creates no depth buffer
        // will spam the debug output with D3D warnings
        // frameBuffer = bgfx::createFrameBuffer(bgfx::BackbufferRatio::Equal, bgfx::TextureFormat::BGRA8);
    }
    this->width = width;
    this->height = height;
}

void Renderer::shutdown()
{
    bgfx::destroy(blitProgram);
    bgfx::destroy(blitSampler);
    bgfx::destroy(frameBuffer);
}

void Renderer::blitToScreen(bgfx::ViewId view)
{
    bgfx::setViewClear(view, BGFX_CLEAR_NONE);
    bgfx::setViewRect(view, 0, 0, width, height);
    bgfx::setViewFrameBuffer(view, BGFX_INVALID_HANDLE);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW);
    bgfx::TextureHandle frameBufferTexture = bgfx::getTexture(frameBuffer);
    bgfx::setTexture(0, blitSampler, frameBufferTexture);
    screenQuad();
    bgfx::submit(view, blitProgram);
}

void Renderer::screenQuad()
{
    constexpr uint32_t vCount = 6;
    constexpr uint32_t iCount = 6;
    constexpr float BOTTOM = -1.0f, TOP = 1.0f;
    constexpr float LEFT = -1.0f, RIGHT = 1.0f;
    static const PosVertex vertices[vCount] = { { BOTTOM, LEFT, 0.0f }, { TOP, LEFT,  0.0f }, { TOP,    RIGHT, 0.0f },
                                                { BOTTOM, LEFT, 0.0f }, { TOP, RIGHT, 0.0f }, { BOTTOM, RIGHT, 0.0f } };
    // clock-wise winding (bgfx default)
    static const uint16_t indices[iCount] = { 0, 1, 2, 3, 4, 5 };

    if(vCount == bgfx::getAvailTransientVertexBuffer(vCount, PosVertex::ms_decl) &&
       iCount == bgfx::getAvailTransientIndexBuffer(iCount))
    {
        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, vCount, PosVertex::ms_decl);
        PosVertex* vData = (PosVertex*)vb.data;

        bgfx::TransientIndexBuffer ib;
        bgfx::allocTransientIndexBuffer(&ib, iCount);
        uint16_t* iData = (uint16_t*)ib.data;

        bx::memCopy(vData, vertices, sizeof(vertices));
        bx::memCopy(iData, indices, sizeof(indices));

        bgfx::setVertexBuffer(0, &vb);
        bgfx::setIndexBuffer(&ib);
    }
    // TODO error out
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
