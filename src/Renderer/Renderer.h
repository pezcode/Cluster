#pragma once

#include <bgfx/bgfx.h>
#include "Renderer/PBRShader.h"
#include "Renderer/LightShader.h"
#include <glm/matrix.hpp>
#include <unordered_map>
#include <string>

class Scene;

class Renderer
{
public:
    Renderer(const Scene* scene);
    virtual ~Renderer() {}

    void initialize();
    void reset(uint16_t width, uint16_t height);
    void render(float dt);
    void shutdown();

    void setVariable(const std::string& name, const std::string& val);

    enum class TonemappingMode : int
    {
        NONE = 0,
        EXPONENTIAL,
        REINHARD,
        REINHARD_LUM,
        HABLE,
        DUIKER,
        ACES,
        ACES_LUM
    };

    void setTonemappingMode(TonemappingMode mode);

    static bool supported();

    // subclasses should override these

    // the first reset happens before initialize
    virtual void onInitialize() {}
    // window resize/flags changed (MSAA, V-Sync, ...)
    virtual void onReset() {}
    virtual void onRender(float dt) = 0;
    virtual void onShutdown() {}

    // buffers for debug output (display in the UI)

    struct TextureBuffer
    {
        bgfx::TextureHandle handle;
        const char* name;
    };

    TextureBuffer* buffers;

    // final output
    // used for tonemapping, screenshots
    bgfx::FrameBufferHandle frameBuffer;

protected:
    struct PosVertex
    {
        float x;
        float y;
        float z;

        static void init()
        {
            layout.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
        }

        static bgfx::VertexLayout layout;
    };

    static constexpr bgfx::ViewId MAX_VIEW = 199; // imgui in bigg uses view 200

    void setViewProjection(bgfx::ViewId view);
    void setNormalMatrix(const glm::mat4& modelMat);

    void blitToScreen(bgfx::ViewId view = MAX_VIEW);

    static bgfx::TextureFormat::Enum findDepthFormat(uint64_t textureFlags, bool stencil = false);
    static bgfx::FrameBufferHandle createFrameBuffer(bool hdr = true, bool depth = true);
    static const char* shaderDir();

    std::unordered_map<std::string, std::string> variables;

    TonemappingMode tonemappingMode;

    const Scene* scene;

    uint16_t width;
    uint16_t height;

    PBRShader pbr;
    LightShader lights;

    uint32_t clearColor;
    float time;

    // set by setViewProjection()
    glm::mat4 viewMat;
    glm::mat4 projMat;

    bgfx::VertexBufferHandle blitTriangleBuffer;

private:
    bgfx::ProgramHandle blitProgram;
    bgfx::UniformHandle blitSampler;
    bgfx::UniformHandle camPosUniform;
    bgfx::UniformHandle normalMatrixUniform;
    bgfx::UniformHandle exposureVecUniform;
    bgfx::UniformHandle tonemappingModeVecUniform;
};
