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
    void setMultipleScattering(bool enabled);
    void setWhiteFurnace(bool enabled);

    static bool supported();
    static const char* shaderDir();

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

    TextureBuffer* buffers = nullptr;

    // final output
    // used for tonemapping
    bgfx::FrameBufferHandle frameBuffer = BGFX_INVALID_HANDLE;

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

    std::unordered_map<std::string, std::string> variables;

    TonemappingMode tonemappingMode = TonemappingMode::NONE;

    const Scene* scene = nullptr;

    uint16_t width = 0;
    uint16_t height = 0;

    PBRShader pbr;
    LightShader lights;

    uint32_t clearColor = 0;
    float time = 0.0f;

    // set by setViewProjection()
    glm::mat4 viewMat = glm::mat4(1.0);
    glm::mat4 projMat = glm::mat4(1.0);

    bgfx::VertexBufferHandle blitTriangleBuffer = BGFX_INVALID_HANDLE;

private:
    bgfx::ProgramHandle blitProgram = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle blitSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle camPosUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle normalMatrixUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle exposureVecUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle tonemappingModeVecUniform = BGFX_INVALID_HANDLE;
};
