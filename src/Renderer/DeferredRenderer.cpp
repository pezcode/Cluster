#include "DeferredRenderer.h"

#include "Scene/Scene.h"
#include <bigg.hpp>
#include <bx/string.h>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

DeferredRenderer::DeferredRenderer(const Scene* scene) :
    Renderer(scene),
    gBufferTextures { { BGFX_INVALID_HANDLE, "Diffuse + roughness" },
                      { BGFX_INVALID_HANDLE, "Normal"              },
                      { BGFX_INVALID_HANDLE, "F0 + metallic"       },
                      { BGFX_INVALID_HANDLE, "Depth"               },
                      { BGFX_INVALID_HANDLE, nullptr               } },
    gBuffer(BGFX_INVALID_HANDLE),
    geometryProgram(BGFX_INVALID_HANDLE),
    lightProgram(BGFX_INVALID_HANDLE)
{
    static_assert(BX_COUNTOF(gBufferTextures) <= (GBufferAttachment::Count + 1), "Number of G-Buffer visualization textures can't exceed attachment count");

    buffers = gBufferTextures;
}

DeferredRenderer::~DeferredRenderer()
{

}

bool DeferredRenderer::supported()
{
    const bgfx::Caps* caps = bgfx::getCaps();
    return Renderer::supported() &&
           // fragment depth available in fragment shader
           (caps->supported & BGFX_CAPS_FRAGMENT_DEPTH) != 0 &&
           // render target for diffuse and material
           (caps->formats[bgfx::TextureFormat::BGRA8]   & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0 &&
           // render target for normals
           (caps->formats[bgfx::TextureFormat::RGB10A2] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0 &&
           // multiple render targets
           caps->limits.maxFBAttachments >= (GBufferAttachment::Count - 1);
}

void DeferredRenderer::onInitialize()
{
    char vsName[128], fsName[128];

    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_deferred_geometry.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_deferred_geometry.bin");
    geometryProgram = bigg::loadProgram(vsName, fsName);

    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_deferred_light.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_deferred_light.bin");
    lightProgram = bigg::loadProgram(vsName, fsName);
}

void DeferredRenderer::onReset()
{
    if(!bgfx::isValid(gBuffer))
    {
        gBuffer = createGBuffer();

        for(size_t i = 0; i < BX_COUNTOF(gBufferTextures); i++)
        {
            gBufferTextures[i].handle = bgfx::getTexture(gBuffer, (uint8_t)i);
        }
    }
}

void DeferredRenderer::onRender(float dt)
{
    enum : bgfx::ViewId
    {
        vGeometry = 0,
        vLight
    };

    const uint32_t BLACK = 0x000000FF;

    bgfx::setViewClear(vGeometry, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, BLACK, 1.0f, 0);
    bgfx::setViewRect(vGeometry, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vGeometry, gBuffer);
    bgfx::touch(vGeometry);

    bgfx::setViewClear(vLight, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0); // is this correct?
    bgfx::setViewRect(vLight, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vLight, frameBuffer);
    bgfx::touch(vLight);

    if(!scene->loaded)
        return;

    bgfx::setViewName(vGeometry, "Deferred geometry pass");

    // render geometry, write to G-Buffer

    setViewProjection(vGeometry);

    uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;

    for(const Mesh& mesh : scene->meshes)
    {
        const Material& mat = scene->materials[mesh.material];
        // transparent materials are rendered in a seperate forward pass
        if(!mat.blend)
        {
            glm::mat4 model = glm::identity<glm::mat4>();
            bgfx::setTransform(glm::value_ptr(model));
            setNormalMatrix(model);
            bgfx::setVertexBuffer(0, mesh.vertexBuffer);
            bgfx::setIndexBuffer(mesh.indexBuffer);
            uint64_t materialState = pbr.bindMaterial(mat);
            bgfx::setState(state | materialState);
            bgfx::submit(vGeometry, geometryProgram);
        }
    }

    bgfx::setViewName(vLight, "Deferred light pass");

    // render lights to framebuffer

    for(const PointLight& light : scene->pointLights.lights)
    {
        //bgfx::submit(vGeometry, lightProgram);
    }
}

void DeferredRenderer::onShutdown()
{
    bgfx::destroy(geometryProgram);
    bgfx::destroy(lightProgram);

    geometryProgram = lightProgram = BGFX_INVALID_HANDLE;
}

bgfx::FrameBufferHandle DeferredRenderer::createGBuffer()
{
    bgfx::TextureHandle textures[GBufferAttachment::Count];

    const uint64_t samplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT |
                                  BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

    bgfx::TextureFormat::Enum attachmentFormats[] =
    {
        bgfx::TextureFormat::BGRA8,
        bgfx::TextureFormat::RGB10A2,
        bgfx::TextureFormat::BGRA8
    };

    for(size_t i = 0; i < GBufferAttachment::Depth; i++)
    {
        assert(bgfx::isTextureValid(0, false, 1, attachmentFormats[i], BGFX_TEXTURE_RT | samplerFlags));
        textures[i] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, attachmentFormats[i], BGFX_TEXTURE_RT | samplerFlags);
    }

    // not write only
    bgfx::TextureFormat::Enum depthFormat = findDepthFormat(BGFX_TEXTURE_RT | samplerFlags);
    assert(depthFormat != bgfx::TextureFormat::Count);
    textures[Depth] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, depthFormat, BGFX_TEXTURE_RT | samplerFlags);

    bgfx::FrameBufferHandle gb = bgfx::createFrameBuffer((uint8_t)GBufferAttachment::Count, textures, true);

    if(!bgfx::isValid(gb))
        Log->error("Failed to create G-Buffer");
    else
        bgfx::setName(gb, "G-Buffer");

    return gb;
}
