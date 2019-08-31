#include "Config.h"

#include <bx/commandline.h>
#include "Renderer/Renderer.h"

Config::Config() :
    writeLog(true),
    logFile("Cluster.log"),
    renderer(bgfx::RendererType::Count), // default renderer, chosen by platform
    renderPath(Cluster::Forward),
    tonemappingMode(Renderer::TonemappingMode::ACES),
    vsync(false),
    //sceneFile("assets/models/duck/Duck.gltf"),
    sceneFile("assets/models/Sponza/glTF/Sponza.gltf"),
    lights(1),
    maxLights(1000),
    movingLights(false),
    fullscreen(false),
    showUI(true),
    showConfigWindow(true),
    showLog(true),
    showStatsOverlay(true),
    overlays({true, true, true}),
    showBuffers(true),
    debugVisualization(false)
{
}

void Config::readArgv(int argc, char* argv[])
{
    bx::CommandLine cmdLine(argc, argv);

    // DX 9.0c (shader model 3.0) doesn't allow indexing into the light buffer
    // so shaders for DX9 aren't even compiled anymore

    // DX11 and OpenGL work
    // DX12 just instantly quits
    // Vulkan? need to update bgfx/bigg

    //cmdLine.hasArg("gl");

    renderer = bgfx::RendererType::OpenGL;
    //renderer = bgfx::RendererType::Direct3D12;

    showStatsOverlay = false;
    showLog = false;
    showBuffers = false;
}
