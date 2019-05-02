#pragma once

#include "Renderer.h"

class DeferredRenderer : public Renderer
{
public:
    DeferredRenderer(const Scene* scene);
    virtual ~DeferredRenderer();

    virtual void initialize() override;
    virtual void reset(uint16_t width, uint16_t height) override;
    virtual void shutdown() override;

    virtual void render(float dt) override;

private:
    TextureBuffer bufferList[4];
};
