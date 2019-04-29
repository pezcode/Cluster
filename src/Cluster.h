#pragma once

#include <bigg.hpp>
#include "UI.h"
#include "Config.h"
#include "Scene.h"
#include "Renderer.h"
#include <memory>

class Cluster : public bigg::Application
{
    friend class ClusterUI;

public:
    Cluster();

    int run(int argc, char** argv);

    // bigg callbacks

    void initialize(int _argc, char* _argv[]) override;
    void onReset() override;
    void onChar(unsigned int codepoint) override;
    void update(float dt) override;
    int shutdown() override;

private:
    class BgfxCallbacks : public bgfx::CallbackI
    {
    public:
        virtual ~BgfxCallbacks() {}
        virtual void fatal(bgfx::Fatal::Enum, const char*) override {}
        virtual void traceVargs(const char*, uint16_t, const char*, va_list) override {}
        virtual void profilerBegin(const char*, uint32_t, const char*, uint16_t) override {}
        virtual void profilerBeginLiteral(const char*, uint32_t, const char*, uint16_t) override {}
        virtual void profilerEnd() override {}
        virtual uint32_t cacheReadSize(uint64_t) override { return 0; }
        virtual bool cacheRead(uint64_t, void*, uint32_t) override { return false; }
        virtual void cacheWrite(uint64_t, const void*, uint32_t) override {}
        virtual void captureBegin(uint32_t, uint32_t, uint32_t, bgfx::TextureFormat::Enum, bool) override {}
        virtual void captureEnd() override {}
        virtual void captureFrame(const void*, uint32_t) override {}

        virtual void screenShot(const char* _filePath,
                                uint32_t _width,
                                uint32_t _height,
                                uint32_t _pitch,
                                const void* _data,
                                uint32_t /*_size*/,
                                bool _yflip) override;
    };

    BgfxCallbacks callbacks;

    Config config;
    ClusterUI ui;
    Scene scene;
    std::unique_ptr<Renderer> renderer;
};
