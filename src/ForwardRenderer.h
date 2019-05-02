#pragma once

#include "Renderer.h"

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(const Scene* scene);
    virtual ~ForwardRenderer();

    virtual void initialize() override;
    virtual void reset(uint16_t width, uint16_t height) override;
    virtual void shutdown() override;

    virtual void render(float dt) override;

private:
    float mTime;

    bgfx::ProgramHandle mProgram;
    bgfx::VertexBufferHandle mVbh;
    bgfx::IndexBufferHandle mIbh;
};
