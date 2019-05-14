#include "Config.h"

Config::Config() :
    writeLog(true),
    logFile("Cluster.log"),
    renderer(bgfx::RendererType::Count),
    renderPath(Cluster::Forward),
    sceneFile("assets/models/Sponza/glTF/Sponza.gltf"), //("assets/models/duck/Duck.gltf"),
    skyColor{ 0.53f, 0.81f, 0.98f }, // https://en.wikipedia.org/wiki/Sky_blue#Light_sky_blue
    lights(0),
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

    // D3D9 doesn't seem to work, device gets lost
    renderer = bgfx::RendererType::Direct3D11;

    showStatsOverlay = false;
    showLog = false;
    showBuffers = false;
    //fullscreen = true;
    lights = 50;
}
