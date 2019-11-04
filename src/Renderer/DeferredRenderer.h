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

    struct PosVertex
    {
        float x;
        float y;
        float z;

        static void init()
        {
            decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .end();
        }

        static bgfx::VertexDecl decl;
    };

    bgfx::VertexBufferHandle pointLightVertexBuffer;
    bgfx::IndexBufferHandle pointLightIndexBuffer;

    enum GBufferAttachment : size_t
    {
        // no world position
        // gl_Fragcoord is enough to unproject

        // RGB = diffuse
        // A = a (remapped roughness)
        Diffuse_A,

        // TODO encode as two 16-bit components (RG16F)
        // https://aras-p.info/texts/CompactNormalStorage.html
        // Method #4: Spheremap Transform looks ideal
        Normal,

        // RGB = F0 (Fresnel at normal incidence)
        // A = metallic
        // TODO? don't use F0, calculate from diffuse and metallic in shader
        F0_Metallic,

        Depth,

        Count
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
    bgfx::ProgramHandle pointLightProgram;
    bgfx::ProgramHandle transparencyProgram;

    static bgfx::FrameBufferHandle createGBuffer();
};
