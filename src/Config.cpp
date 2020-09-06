#include "Config.h"

#include <bx/commandline.h>
#include "Renderer/Renderer.h"

Config::Config() :
    writeLog(true),
    logFile("Cluster.log"),
    renderer(bgfx::RendererType::Count), // default renderer, chosen by platform
    renderPath(Cluster::RenderPath::Clustered),
    tonemappingMode(Renderer::TonemappingMode::ACES),
    profile(true),
    vsync(false),
    sceneFile("assets/models/Sponza/glTF/Sponza.gltf"),
    customScene(false),
    lights(1),
    maxLights(3000),
    movingLights(false),
    fullscreen(false),
    showUI(true),
    showConfigWindow(true),
    showLog(false),
    showStatsOverlay(false),
    overlays({ true, true, true, true }),
    showBuffers(false),
    debugVisualization(false)
{
}

void Config::readArgv(int argc, char* argv[])
{
    // argv must outlive Config
    // we store pointers into argv for the scene file
    bx::CommandLine cmdLine(argc, argv);

    if(cmdLine.hasArg("noop"))
        renderer = bgfx::RendererType::Noop;
	else if(cmdLine.hasArg("gl"))
        renderer = bgfx::RendererType::OpenGL;
    else if(cmdLine.hasArg("vk"))
        renderer = bgfx::RendererType::Vulkan;
    // missing required features
    //else if(cmdLine.hasArg("d3d9"))
    //    renderer = bgfx::RendererType::Direct3D9;
    else if(cmdLine.hasArg("d3d11"))
        renderer = bgfx::RendererType::Direct3D11;
    else if(cmdLine.hasArg("d3d12"))
        renderer = bgfx::RendererType::Direct3D12;
    // not tested
    //else if(cmdLine.hasArg("mtl"))
    //    renderer = bgfx::RendererType::Metal;

    const char* scene = cmdLine.findOption("scene");
    if(scene)
    {
        sceneFile = scene;
        customScene = true;
    }
}
