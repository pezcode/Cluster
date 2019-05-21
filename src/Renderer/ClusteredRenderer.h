#pragma once

#include "Renderer.h"

class ClusteredRenderer : public Renderer
{
public:
    ClusteredRenderer(const Scene* scene);
    virtual ~ClusteredRenderer();

    static bool supported();

    virtual void onInitialize() override;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;

private:
    static const uint32_t CLUSTER_WIDTH  = 32;
    static const uint32_t CLUSTER_HEIGHT = 32;

    bgfx::ProgramHandle lightingProgram;
    bgfx::ProgramHandle lightCullingComputeProgram;
};
