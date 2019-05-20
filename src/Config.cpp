#include "Config.h"

Config::Config() :
    writeLog(true),
    logFile("Cluster.log"),
    renderer(bgfx::RendererType::Count),
    renderPath(Cluster::Forward),
    vsync(false),
    //sceneFile("assets/models/duck/Duck.gltf"),
    sceneFile("assets/models/Sponza/glTF/Sponza.gltf"),
    lights(1),
    fullscreen(false),
    showUI(true),
    showConfigWindow(true),
    showLog(true),
    showStatsOverlay(true),
    overlays({true, true, true}),
    showBuffers(true)
{
}

void Config::readArgv(int argc, char* argv[])
{
    // bgfx OpenGL renderer uses version 2.1
    // can be built with BGFX_CONFIG_RENDERER_OPENGL=43
    // this adds compute shaders and texture copy
    // somehow this also makes it the only usable renderer
    // passing D3D11 as renderer will still use OpenGL with that define

    // DX 9.0c (shader model 3.0) doesn't allow indexing into the light buffer
    // so shaders for DX9 aren't even compiled anymore

    // DX11, DX12, OpenGL all work
    // Vulkan doesn't support framebuffer textures

    renderer = bgfx::RendererType::Direct3D11;

    showStatsOverlay = false;
    showLog = false;
    showBuffers = false;
    //fullscreen = true;
    lights = 50;
}
