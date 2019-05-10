#pragma once

#include "Renderer.h"

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(const Scene* scene);
    virtual ~ForwardRenderer();

    static bool supported();

    virtual void onInitialize() override;
    virtual void onReset() override;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;

private:
    bgfx::ProgramHandle program;
    bgfx::UniformHandle baseColorUniform;
    bgfx::UniformHandle metallicRoughnessUniform;
    bgfx::UniformHandle hasTexturesUniform;
    bgfx::UniformHandle baseColorSampler;
    bgfx::UniformHandle metallicRoughnessSampler;
    bgfx::UniformHandle normalSampler;

    void bindMaterial(const Scene::Material& material);
};
