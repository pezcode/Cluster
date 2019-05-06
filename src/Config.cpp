#include "Config.h"

Config::Config() :
    writeLog(true),
    logFile("Cluster.log"),
    renderer(bgfx::RendererType::Count),
    renderPath(Cluster::Forward),
    sceneFile("assets/models/duck/Duck.gltf"),
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
    // TODO
    // set renderer
    // load scene
    // consider using Argh! (https://github.com/adishavit/argh)

    // bgfx is built with BGFX_CONFIG_RENDERER_OPENGL=43
    // this adds compute shaders and texture copy
    // somehow this also makes it the only usable renderer
    // TODO keep DX11 as default on Windows but raise used OpenGL version?
    // define something else so it uses both?
    // renderer = bgfx::RendererType::OpenGL;

    // this doesn't work, still use OpenGL
    renderer = bgfx::RendererType::Direct3D11;

    showStatsOverlay = false;
    showLog = false;
    showBuffers = false;
    //fullscreen = true;
    lights = 50;
}
