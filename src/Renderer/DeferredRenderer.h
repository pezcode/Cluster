#pragma once

#include "Renderer.h"

class DeferredRenderer : public Renderer
{
public:
    DeferredRenderer(const Scene* scene);
    virtual ~DeferredRenderer();

    static bool supported();

    virtual void onInitialize() override;
    virtual void onReset() override;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;

private:
    bgfx::VertexBufferHandle pointLightVertexBuffer;
    bgfx::IndexBufferHandle pointLightIndexBuffer;

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
        F0_Metallic,

        // RGB = emissive radiance
        // A = occlusion multiplier
        EmissiveOcclusion,

        Depth,

        Count
    };

    static constexpr bgfx::TextureFormat::Enum gBufferAttachmentFormats[GBufferAttachment::Count - 1] =
    {
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
    bgfx::FrameBufferHandle gBuffer;

    bgfx::TextureHandle lightDepthTexture;
    bgfx::FrameBufferHandle accumFrameBuffer;

    bgfx::UniformHandle lightIndexVecUniform;

    bgfx::ProgramHandle geometryProgram;
    bgfx::ProgramHandle fullscreenProgram;
    bgfx::ProgramHandle pointLightProgram;
    bgfx::ProgramHandle transparencyProgram;

    static bgfx::FrameBufferHandle createGBuffer();
    void bindGBuffer();
};
