#pragma once

#include "Renderer.h"
#include "ClusterShader.h"

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
    bgfx::ProgramHandle clusterBuildingComputeProgram;
    bgfx::ProgramHandle lightCullingComputeProgram;
    bgfx::ProgramHandle lightingProgram;

    ClusterShader clusters;
};
