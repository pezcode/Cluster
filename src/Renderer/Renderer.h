#pragma once

#include <bgfx/bgfx.h>
#include "PBRShader.h"

class Scene;

class Renderer
{
public:

    Renderer(const Scene* scene);
    virtual ~Renderer();

    void initialize();
    void reset(uint16_t width, uint16_t height);
    void render(float dt);
    void shutdown();

    static bool supported();

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
    // used for tonemapping, screenshots
    bgfx::FrameBufferHandle frameBuffer;

protected:

    struct PosTexCoord0Vertex
    {
        float x;
        float y;
        float z;

        float u;
        float v;

        static void init()
        {
            decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();
        }

        static bgfx::VertexDecl decl;
    };

    static constexpr bgfx::ViewId MAX_VIEW = 199; // imgui in bigg uses view 200

    void blitToScreen(bgfx::ViewId view = MAX_VIEW);

    static bgfx::FrameBufferHandle createFrameBuffer(uint16_t width, uint16_t height, bool hdr = true, bool depth = true);
    static const char* shaderDir();

    const Scene* scene;
    uint16_t width;
    uint16_t height;
    PBRShader pbr;
    uint32_t clearColor;
    float time;

private:

    bgfx::ProgramHandle blitProgram;
    bgfx::UniformHandle blitSampler;
    bgfx::VertexBufferHandle quadVB;
};
