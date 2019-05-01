#include "Config.h"

Config::Config() :
    renderer(bgfx::RendererType::Count),
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
    // somehow this also makes it the default
    // TODO keep DX11 as default on Windows but raise used OpenGL version?
    // renderer = bgfx::RendererType::OpenGL;
}
