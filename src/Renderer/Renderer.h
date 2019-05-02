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

    void initialize();
    void reset(uint16_t width, uint16_t height);
    void render(float dt);
    void shutdown();

    // subclasses should override these

    // the first reset happens before initialize
    virtual void onInitialize() {}
    // window resize/flags changed (MSAA, VSYNC, ...)
    virtual void onReset() {}
    virtual void onRender(float dt) = 0;
    virtual void onShutdown() {}

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

    static constexpr bgfx::ViewId MAX_VIEW = 199; // imgui in bigg uses view 200

    void blitToScreen(bgfx::ViewId view = MAX_VIEW);
    void screenQuad();
    static const char* shaderDir();

    const Scene* scene;
    uint16_t width;
    uint16_t height;

private:

    bgfx::ProgramHandle blitProgram;
    bgfx::UniformHandle blitSampler;
};
