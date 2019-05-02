#pragma once

#include "Renderer.h"

class ClusteredRenderer : public Renderer
{
public:
    ClusteredRenderer(const Scene* scene);
    virtual ~ClusteredRenderer();

    virtual void initialize() override;
    virtual void reset(uint16_t width, uint16_t height) override;
    virtual void shutdown() override;

    virtual void render(float dt) override;
};
