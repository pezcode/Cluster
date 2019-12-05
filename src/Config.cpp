#include "Config.h"

#include <bx/commandline.h>
#include "Renderer/Renderer.h"

Config::Config() :
    writeLog(true),
    logFile("Cluster.log"),
    renderer(bgfx::RendererType::Count), // default renderer, chosen by platform
    renderPath(Cluster::RenderPath::Forward),
    tonemappingMode(Renderer::TonemappingMode::ACES),
    profile(false),
    vsync(false),
    msaa(false),
    //sceneFile("assets/models/BoomBox/glTF/BoomBox.gltf"),
    //sceneFile("assets/models/Duck/glTF/Duck.gltf"),
    //sceneFile("assets/models/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf"),
    sceneFile("assets/models/Sponza/glTF/Sponza.gltf"),
    lights(1),
    maxLights(1000),
    movingLights(false),
    fullscreen(false),
    showUI(true),
    showConfigWindow(true),
    showLog(true),
    showStatsOverlay(true),
    overlays({ true, true, true, true }),
    showBuffers(true),
    debugVisualization(false)
{
}

void Config::readArgv(int argc, char* argv[])
{
    bx::CommandLine cmdLine(argc, argv);

    // D3D 9.0c (shader model 3.0) doesn't allow indexing into the light buffer
    // so shaders for D3D9 aren't even compiled anymore

    // D3D11, D3D12, OpenGL work
    // Vulkan? need to update bgfx/bigg

    //cmdLine.hasArg("gl");

    renderer = bgfx::RendererType::OpenGL;
    //renderer = bgfx::RendererType::Direct3D12;

    profile = true;

    showStatsOverlay = false;
    showLog = false;
    showBuffers = false;

    msaa = true;
}
