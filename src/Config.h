#pragma once

#include <bgfx/bgfx.h>
#include "Cluster.h"

class Config
{
public:
    Config();

    bgfx::RendererType::Enum renderer;
    Cluster::RenderPath renderPath;

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
