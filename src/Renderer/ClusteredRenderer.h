#pragma once

#include "Renderer.h"
#include "ClusterShader.h"

class ClusteredRenderer : public Renderer
{
public:
    ClusteredRenderer(const Scene* scene);

    static bool supported();

    virtual void onInitialize() override;
    virtual void onRender(float dt) override;
    virtual void onShutdown() override;

private:
    glm::mat4 oldProjMat;

    bgfx::ProgramHandle clusterBuildingComputeProgram;
    bgfx::ProgramHandle resetCounterComputeProgram;
    bgfx::ProgramHandle lightCullingComputeProgram;
    bgfx::ProgramHandle lightingProgram;
    bgfx::ProgramHandle debugVisProgram;

    ClusterShader clusters;
};
