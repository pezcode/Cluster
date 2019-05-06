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
    float mTime;

    bgfx::ProgramHandle program;
};
