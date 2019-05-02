#pragma once

#include <bgfx/bgfx.h>
#include "Cluster.h"

class Config
{
public:
    Config();

    // Renderer

    bgfx::RendererType::Enum renderer;
    Cluster::RenderPath renderPath;

    // Scene

    int lights;

    // UI

    bool fullscreen;
    bool showUI;
    bool showConfigWindow;
    bool showLog;
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
