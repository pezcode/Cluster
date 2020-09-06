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
    glm::mat4 oldProjMat = glm::mat4(0.0f);

    bgfx::ProgramHandle clusterBuildingComputeProgram = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle resetCounterComputeProgram = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle lightCullingComputeProgram = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle lightingProgram = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle debugVisProgram = BGFX_INVALID_HANDLE;

    ClusterShader clusters;
};
