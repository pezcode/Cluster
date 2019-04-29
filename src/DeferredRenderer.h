#pragma once

#include "Renderer.h"

class DeferredRenderer : public Renderer
{
public:

    DeferredRenderer(const Scene* scene);

    virtual void render(float dt) override;

private:
    TextureBuffer bufferList[4];
};
