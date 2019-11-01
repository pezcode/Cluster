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

    enum GBufferAttachment : size_t
    {
        // no position
        // gl_Fragcoord is enough to unproject

        // RGB = diffuse
        // A = a (remapped roughness)
        Diffuse_A,

        // TODO encode as two 16-bit components (RG16F)
        // https://aras-p.info/texts/CompactNormalStorage.html
        // Method #4: Spheremap Transform looks ideal
        Normal,

        // PBR material
        // RGB = F0 (Fresnel at normal incidence)
        // A = metallic
        Metallic_F0,

        Depth,

        Count
    };

    TextureBuffer gBufferTextures[GBufferAttachment::Count + 1];
    bgfx::FrameBufferHandle gBuffer;

    bgfx::ProgramHandle geometryProgram;
    bgfx::ProgramHandle lightProgram;

    static bgfx::FrameBufferHandle createGBuffer();
};
