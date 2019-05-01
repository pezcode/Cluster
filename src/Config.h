#pragma once

#include <bgfx/bgfx.h>

class Config
{
public:
    Config();

    bgfx::RendererType::Enum renderer;

    bool fullscreen;
    bool showUI;
    bool showConfigWindow;
    bool showStatsOverlay;
    struct
    {
        bool fps;
        bool frameTime;
        bool gpuMemory;
    } overlays;
    bool showBuffers;

    void readArgv(int argc, char* argv[]);
};
