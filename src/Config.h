#pragma once

#include <bgfx/bgfx.h>
#include "Renderer/Renderer.h"
#include "Cluster.h"

class Config
{
public:
    Config();

    void readArgv(int argc, char* argv[]);

    // * = not exposed to UI

    // Log

    bool writeLog;       // *
    const char* logFile; // *

    // Renderer

    bgfx::RendererType::Enum renderer; // *
    Cluster::RenderPath renderPath;
    Renderer::TonemappingMode tonemappingMode;

    bool profile; // enable bgfx view profiling *
    bool vsync;   // *

    // Scene

    const char* sceneFile; // gltf file to load *
    bool customScene; // not the standard Sponza scene, don't place debug lights/camera *
    int lights;
    int maxLights; // *
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
