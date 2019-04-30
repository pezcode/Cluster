#include "Renderer.h"

#include <bigg.hpp>
#include <bx/macros.h>
#include <bx/string.h>

bgfx::VertexDecl PosVertex::ms_decl;

void Renderer::initialize()
{
    frameBuffer = bgfx::createFrameBuffer(bgfx::BackbufferRatio::Equal, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_READ_BACK);
    PosVertex::init();

    char vsName[32];
    char fsName[32];

    const char* dir = shaderDir();

    bx::strCopy(vsName, BX_COUNTOF(vsName), dir);
    bx::strCat(vsName, BX_COUNTOF(vsName), "vs_screen_quad.bin");

    bx::strCopy(fsName, BX_COUNTOF(fsName), dir);
    bx::strCat(fsName, BX_COUNTOF(fsName), "fs_screen_quad.bin");

    blitProgram = bigg::loadProgram(vsName, fsName);

    blitSampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
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
    bgfx::destroy(blitProgram);
    bgfx::destroy(blitSampler);
    bgfx::destroy(frameBuffer);
}

void Renderer::blitToScreen(bgfx::ViewId view)
{
    bgfx::setViewClear(view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030FF, 1.0f, 0);
    //bgfx::setViewClear(view, BGFX_CLEAR_DISCARD_DEPTH | BGFX_CLEAR_DISCARD_STENCIL);
    bgfx::setViewRect(view, 0, 0, width, height);
    bgfx::setViewFrameBuffer(view, BGFX_INVALID_HANDLE);

    bgfx::TextureHandle frameBufferTexture = bgfx::getTexture(frameBuffer); // no need to destroy (see example 09)
    bgfx::setTexture(0, blitSampler, frameBufferTexture);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    screenQuad();
    bgfx::submit(view, blitProgram);
}

void Renderer::screenQuad()
{
    static const PosVertex vertices[6] = {
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 1.0f, 1.0f }
    };
    static const uint32_t indices[6] = { 0, 1, 2, 3, 4, 5 };

    size_t vCount = BX_COUNTOF(vertices);
    size_t iCount = BX_COUNTOF(indices);

    if(vCount == bgfx::getAvailTransientVertexBuffer(vCount, PosVertex::ms_decl) &&
       iCount == bgfx::getAvailTransientIndexBuffer(iCount))
    {
        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, vCount, PosVertex::ms_decl);
        PosVertex* vData = (PosVertex*)vb.data;

        bgfx::TransientIndexBuffer ib;
        bgfx::allocTransientIndexBuffer(&ib, iCount);
        uint32_t* iData = (uint32_t*)ib.data;        

        bx::memCopy(vData, vertices, sizeof(vertices));
        bx::memCopy(iData, indices, sizeof(indices));

        /*
        bgfx::TransientIndexBuffer ib;
        bgfx::allocTransientIndexBuffer(&ib, 6);

        const float zz = 0.0f;

        float _textureWidth = width;
        float _textureHeight = height;

        float _width = 1.0f;
        float _height = 1.0f;

        const float minx = -_width;
        const float maxx = _width;
        const float miny = 0.0f;
        const float maxy = _height * 2.0f;

        float s_texelHalf = bgfx::RendererType::Direct3D9 == bgfx::getCaps()->rendererType ? 0.5f : 0.0f;

        const float texelHalfW = s_texelHalf / _textureWidth;
        const float texelHalfH = s_texelHalf / _textureHeight;
        const float minu = -1.0f + texelHalfW;
        const float maxu = 1.0f + texelHalfW;

        float minv = texelHalfH;
        float maxv = 2.0f + texelHalfH;

        if(bgfx::getCaps()->originBottomLeft)
        {
            float temp = minv;
            minv = maxv;
            maxv = temp;

            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = zz;
        vertex[0].m_rgba = 0xffffffff;
        vertex[0].m_u = minu;
        vertex[0].m_v = minv;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = zz;
        vertex[1].m_rgba = 0xffffffff;
        vertex[1].m_u = maxu;
        vertex[1].m_v = minv;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = zz;
        vertex[2].m_rgba = 0xffffffff;
        vertex[2].m_u = maxu;
        vertex[2].m_v = maxv;
        */

        bgfx::setVertexBuffer(0, &vb);
        bgfx::setIndexBuffer(&ib);
    }
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
