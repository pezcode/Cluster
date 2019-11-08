#pragma once

#include <bgfx/bgfx.h>
#include "Renderer/Renderer.h"
#include "Cluster.h"

class Config
{
public:
    Config();

    void readArgv(int argc, char* argv[]);

    // Log

    bool writeLog; // not exposed to UI
    const char* logFile; // not exposed to UI

    // Renderer

    bgfx::RendererType::Enum renderer; // not exposed to UI
    Cluster::RenderPath renderPath;
    Renderer::TonemappingMode tonemappingMode;

    bool profile; // not exposed to UI
    bool vsync; // not exposed to UI
    bool msaa; // not exposed to UI

    // Scene

    const char* sceneFile; // not exposed to UI
    int lights;
    int maxLights; // not exposed to UI
    bool movingLights;

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
        bool profiler;
        bool gpuMemory;
    } overlays;

    bool showBuffers;
    // clustered renderer
    bool debugVisualization;
};
