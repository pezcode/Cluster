#include "Renderer.h"

#include "Scene/Scene.h"
#include <bigg.hpp>
#include <bx/macros.h>
#include <bx/string.h>
#include <bx/math.h>
#include <glm/common.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_operation.hpp>

bgfx::VertexLayout Renderer::PosVertex::layout;

Renderer::Renderer(const Scene* scene) :
    buffers(nullptr),
    frameBuffer(BGFX_INVALID_HANDLE),
    tonemappingMode(TonemappingMode::NONE),
    scene(scene),
    width(0),
    height(0),
    clearColor(0),
    time(0.0f),
    viewMat(1.0f),
    projMat(1.0f),
    blitTriangleBuffer(BGFX_INVALID_HANDLE),
    blitProgram(BGFX_INVALID_HANDLE),
    blitSampler(BGFX_INVALID_HANDLE),
    camPosUniform(BGFX_INVALID_HANDLE),
    normalMatrixUniform(BGFX_INVALID_HANDLE),
    exposureVecUniform(BGFX_INVALID_HANDLE),
    tonemappingModeVecUniform(BGFX_INVALID_HANDLE)
{
}

void Renderer::initialize()
{
    PosVertex::init();

    blitSampler = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    camPosUniform = bgfx::createUniform("u_camPos", bgfx::UniformType::Vec4);
    normalMatrixUniform = bgfx::createUniform("u_normalMatrix", bgfx::UniformType::Mat3);
    exposureVecUniform = bgfx::createUniform("u_exposureVec", bgfx::UniformType::Vec4);
    tonemappingModeVecUniform = bgfx::createUniform("u_tonemappingModeVec", bgfx::UniformType::Vec4);

    // triangle used for blitting
    constexpr float BOTTOM = -1.0f, TOP = 3.0f, LEFT = -1.0f, RIGHT = 3.0f;
    const PosVertex vertices[3] = { { LEFT, BOTTOM, 0.0f }, { RIGHT, BOTTOM, 0.0f }, { LEFT, TOP, 0.0f } };
    blitTriangleBuffer = bgfx::createVertexBuffer(bgfx::copy(&vertices, sizeof(vertices)), PosVertex::layout);

    char vsName[128], fsName[128];
    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_tonemap.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_tonemap.bin");
    blitProgram = bigg::loadProgram(vsName, fsName);

    pbr.initialize();
    lights.initialize();

    onInitialize();
}

void Renderer::reset(uint16_t width, uint16_t height)
{
    if(!bgfx::isValid(frameBuffer))
    {
        frameBuffer = createFrameBuffer(true, true);
        bgfx::setName(frameBuffer, "Render framebuffer (pre-postprocessing)");
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
        glm::vec4 camPos = glm::vec4(scene->camera.position(), 1.0f);
        bgfx::setUniform(camPosUniform, glm::value_ptr(camPos));

        // tonemapping expects linear colors
        glm::vec3 linear = glm::convertSRGBToLinear(scene->skyColor);
        glm::u8vec3 result = glm::u8vec3(glm::round(glm::clamp(linear, 0.0f, 1.0f) * 255.0f));
        clearColor = (result[0] << 24) | (result[1] << 16) | (result[2] << 8) | 255;
    }
    else
    {
        // gray
        clearColor = 0x303030FF;
    }

    onRender(dt);
    blitToScreen(MAX_VIEW);

    // bigg doesn't do this
    bgfx::setViewName(MAX_VIEW + 1, "imgui");
}

void Renderer::shutdown()
{
    onShutdown();

    pbr.shutdown();
    lights.shutdown();

    bgfx::destroy(blitProgram);
    bgfx::destroy(blitSampler);
    bgfx::destroy(camPosUniform);
    bgfx::destroy(normalMatrixUniform);
    bgfx::destroy(exposureVecUniform);
    bgfx::destroy(tonemappingModeVecUniform);
    bgfx::destroy(blitTriangleBuffer);
    if(bgfx::isValid(frameBuffer))
        bgfx::destroy(frameBuffer);

    blitProgram = BGFX_INVALID_HANDLE;
    blitSampler = camPosUniform = normalMatrixUniform = exposureVecUniform = tonemappingModeVecUniform =
        BGFX_INVALID_HANDLE;
    blitTriangleBuffer = BGFX_INVALID_HANDLE;
    frameBuffer = BGFX_INVALID_HANDLE;
}

void Renderer::setVariable(const std::string& name, const std::string& val)
{
    variables[name] = val;
}

void Renderer::setTonemappingMode(TonemappingMode mode)
{
    tonemappingMode = mode;
}

bool Renderer::supported()
{
    const bgfx::Caps* caps = bgfx::getCaps();
    return
        // SDR color attachment
        (caps->formats[bgfx::TextureFormat::BGRA8] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0 &&
        // HDR color attachment
        (caps->formats[bgfx::TextureFormat::RGBA16F] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0;
}

void Renderer::setViewProjection(bgfx::ViewId view)
{
    // view matrix
    viewMat = scene->camera.matrix();
    // projection matrix
    bx::mtxProj(glm::value_ptr(projMat),
                scene->camera.fov,
                float(width) / height,
                scene->camera.zNear,
                scene->camera.zFar,
                bgfx::getCaps()->homogeneousDepth,
                bx::Handness::Left);
    bgfx::setViewTransform(view, glm::value_ptr(viewMat), glm::value_ptr(projMat));
}

void Renderer::setNormalMatrix(const glm::mat4& modelMat)
{
    // usually the normal matrix is based on the model view matrix
    // but shading is done in world space (not eye space) so it's just the model matrix
    //glm::mat4 modelViewMat = viewMat * modelMat;

    // if we don't do non-uniform scaling, the normal matrix is the same as the model-view matrix
    // (only the magnitude of the normal is changed, but we normalize either way)
    //glm::mat3 normalMat = glm::mat3(modelMat);

    // use adjugate instead of inverse
    // see https://github.com/graphitemaster/normals_revisited#the-details-of-transforming-normals
    // cofactor is the transpose of the adjugate
    glm::mat3 normalMat = glm::transpose(glm::adjugate(glm::mat3(modelMat)));
    bgfx::setUniform(normalMatrixUniform, glm::value_ptr(normalMat));
}

void Renderer::blitToScreen(bgfx::ViewId view)
{
    bgfx::setViewName(view, "Tonemapping");
    bgfx::setViewClear(view, BGFX_CLEAR_NONE);
    bgfx::setViewRect(view, 0, 0, width, height);
    bgfx::setViewFrameBuffer(view, BGFX_INVALID_HANDLE);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_CULL_CW);
    bgfx::TextureHandle frameBufferTexture = bgfx::getTexture(frameBuffer, 0);
    bgfx::setTexture(0, blitSampler, frameBufferTexture);
    float exposureVec[4] = { scene->loaded ? scene->camera.exposure : 1.0f };
    bgfx::setUniform(exposureVecUniform, exposureVec);
    float tonemappingModeVec[4] = { (float)tonemappingMode };
    bgfx::setUniform(tonemappingModeVecUniform, tonemappingModeVec);
    bgfx::setVertexBuffer(0, blitTriangleBuffer);
    bgfx::submit(view, blitProgram);
}

bgfx::TextureFormat::Enum Renderer::findDepthFormat(uint64_t textureFlags, bool stencil)
{
    const bgfx::TextureFormat::Enum depthFormats[] = { bgfx::TextureFormat::D16, bgfx::TextureFormat::D32 };

    const bgfx::TextureFormat::Enum depthStencilFormats[] = { bgfx::TextureFormat::D24S8 };

    const bgfx::TextureFormat::Enum* formats = stencil ? depthStencilFormats : depthFormats;
    size_t count = stencil ? BX_COUNTOF(depthStencilFormats) : BX_COUNTOF(depthFormats);

    bgfx::TextureFormat::Enum depthFormat = bgfx::TextureFormat::Count;
    for(size_t i = 0; i < count; i++)
    {
        if(bgfx::isTextureValid(0, false, 1, formats[i], textureFlags))
        {
            depthFormat = formats[i];
            break;
        }
    }

    assert(depthFormat != bgfx::TextureFormat::Enum::Count);

    return depthFormat;
}

bgfx::FrameBufferHandle Renderer::createFrameBuffer(bool hdr, bool depth)
{
    bgfx::TextureHandle textures[2];
    uint8_t attachments = 0;

    // BGFX_TEXTURE_READ_BACK is not supported for render targets?
    // TODO try blitting for screenshots (new texture with BGFX_TEXTURE_BLIT_DST and BGFX_TEXTURE_READ_BACK)

    const uint64_t samplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT |
                                  BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

    bgfx::TextureFormat::Enum format =
        hdr ? bgfx::TextureFormat::RGBA16F : bgfx::TextureFormat::BGRA8; // BGRA is often faster (internal GPU format)
    assert(bgfx::isTextureValid(0, false, 1, format, BGFX_TEXTURE_RT | samplerFlags));
    textures[attachments++] =
        bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, format, BGFX_TEXTURE_RT | samplerFlags);

    if(depth)
    {
        bgfx::TextureFormat::Enum depthFormat = findDepthFormat(BGFX_TEXTURE_RT_WRITE_ONLY | samplerFlags);
        assert(depthFormat != bgfx::TextureFormat::Enum::Count);
        textures[attachments++] = bgfx::createTexture2D(
            bgfx::BackbufferRatio::Equal, false, 1, depthFormat, BGFX_TEXTURE_RT_WRITE_ONLY | samplerFlags);
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
            path = "shaders/spirv/";
            break;
        default:
            assert(false);
            break;
    }

    return path;
}
