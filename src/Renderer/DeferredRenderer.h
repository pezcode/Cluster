#pragma once

#include "Renderer.h"

class DeferredRenderer : public Renderer
{
public:
    DeferredRenderer(const Scene* scene);

    static bool supported();

    virtual void onInitialize() override;
    virtual void onReset() override;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;

private:
    bgfx::VertexBufferHandle pointLightVertexBuffer = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle pointLightIndexBuffer = BGFX_INVALID_HANDLE;

    enum GBufferAttachment : size_t
    {
        // no world position
        // gl_Fragcoord is enough to unproject

        // RGB = diffuse
        // A = a (remapped roughness)
        Diffuse_A,

        // RG = encoded normal
        Normal,

        // RGB = F0 (Fresnel at normal incidence)
        // A = metallic
        // TODO? don't use F0, calculate from diffuse and metallic in shader
        //       where do we store metallic?
        F0_Metallic,

        // RGB = emissive radiance
        // A = occlusion multiplier
        EmissiveOcclusion,

        Depth,

        Count
    };

    static constexpr uint64_t gBufferSamplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
                                                    BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP |
                                                    BGFX_SAMPLER_V_CLAMP;

    static constexpr bgfx::TextureFormat::Enum gBufferAttachmentFormats[GBufferAttachment::Count - 1] = {
        bgfx::TextureFormat::BGRA8,
        bgfx::TextureFormat::RG16F,
        bgfx::TextureFormat::BGRA8,
        bgfx::TextureFormat::BGRA8
        // depth format is determined dynamically
    };

    TextureBuffer gBufferTextures[GBufferAttachment::Count + 1]; // includes depth, + null-terminated
    uint8_t gBufferTextureUnits[GBufferAttachment::Count];
    const char* gBufferSamplerNames[GBufferAttachment::Count];
    bgfx::UniformHandle gBufferSamplers[GBufferAttachment::Count];
    bgfx::FrameBufferHandle gBuffer = BGFX_INVALID_HANDLE;

    bgfx::TextureHandle lightDepthTexture = BGFX_INVALID_HANDLE;
    bgfx::FrameBufferHandle accumFrameBuffer = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle lightIndexVecUniform = BGFX_INVALID_HANDLE;

    bgfx::ProgramHandle geometryProgram = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle fullscreenProgram = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle pointLightProgram = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle transparencyProgram = BGFX_INVALID_HANDLE;

    static bgfx::FrameBufferHandle createGBuffer();
    void bindGBuffer();
};
