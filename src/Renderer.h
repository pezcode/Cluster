#pragma once

#include "Scene.h"
#include <bgfx/bgfx.h>

class Renderer
{
public:

    Renderer(const Scene* scene) : scene(scene) {}

    virtual void initialize();
    virtual void reset(uint16_t width, uint16_t height);
    virtual void shutdown();

    virtual void render(float dt) = 0;

    struct TextureBuffer
    {
        bgfx::TextureHandle handle;
        const char* name;
    };

    TextureBuffer* buffers = nullptr;

    // final output
    // used for postprocessing, screenshots
    bgfx::FrameBufferHandle frameBuffer;

protected:

    static const char* shaderDir();

    const Scene* scene;
    uint16_t width;
    uint16_t height;
};
