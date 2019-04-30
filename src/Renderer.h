#pragma once

#include "Scene.h"
#include <bgfx/bgfx.h>

struct PosVertex
{
    float m_x;
    float m_y;
    float m_z;

    static void init()
    {
        ms_decl.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .end();
    }

    static bgfx::VertexDecl ms_decl;
};

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

    void blitToScreen(bgfx::ViewId view = 199);

    void screenQuad();

    static const char* shaderDir();

    const Scene* scene;
    uint16_t width;
    uint16_t height;

private:

    bgfx::ProgramHandle blitProgram;
    bgfx::UniformHandle blitSampler;
};
