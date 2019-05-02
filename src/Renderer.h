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

    Renderer(const Scene* scene);
    virtual ~Renderer();

    // subclasses should call these from within their overrides

    // the first reset happens before initialize
    virtual void initialize();
    virtual void reset(uint16_t width, uint16_t height); // window resize/flags changed (MSAA, VSYNC, ...)
    virtual void shutdown();

    virtual void render(float dt) = 0;

    // buffers for debug output (display in the UI)

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
