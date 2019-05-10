#pragma once

#include <bgfx/bgfx.h>
#include "Cluster.h"

class Config
{
public:
    Config();

    // Log

    bool writeLog; // not exposed to UI
    const char* logFile; // not exposed to UI

    // Renderer

    bgfx::RendererType::Enum renderer; // not exposed to UI
    Cluster::RenderPath renderPath;

    // Scene

    const char* sceneFile; // not exposed to UI
    float skyColor[3]; // RGB, not exposed to UI
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
