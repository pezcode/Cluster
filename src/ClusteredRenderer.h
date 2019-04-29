#pragma once

#include "Renderer.h"

class ClusteredRenderer : public Renderer
{
public:

    ClusteredRenderer(const Scene* scene);

    virtual void render(float dt) override;
};
