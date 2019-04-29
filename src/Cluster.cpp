#include "Cluster.h"

#include "ForwardRenderer.h"
#include "DeferredRenderer.h"
#include "ClusteredRenderer.h"

#include <bx/file.h>
#include <bx/string.h>
#include <bimg/bimg.h>

Cluster::Cluster() :
    bigg::Application("Cluster", 1024, 768),
    ui(*this),
    renderer(std::make_unique<ForwardRenderer>(&scene))
{
}

int Cluster::run(int argc, char** argv)
{
    config.readArgv(argc, argv);
    return Application::run(argc, argv, bgfx::RendererType::Count, BGFX_PCI_ID_NONE, 0, &callbacks, nullptr);
}

void Cluster::initialize(int _argc, char* _argv[])
{
    reset(BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X8 | BGFX_RESET_MAXANISOTROPY);
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    renderer->initialize();

    // TESTING
    if(renderer->buffers)
    {
        for(size_t i = 0; renderer->buffers[i].name != nullptr; i++)
        {
            renderer->buffers[i].handle = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, bgfx::TextureFormat::RGBA8);
        }
    }

    ui.initialize();
}

void Cluster::onReset()
{
    // TODO destroy buffers etc
    renderer->reset(uint16_t(getWidth()), uint16_t(getHeight()));

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030FF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t(getHeight()));
}

void Cluster::onChar(unsigned int codepoint)
{
    switch(codepoint)
    {
        case 'r':
            config.showUI = true;
            config.showConfigWindow = true;
            break;
    }
}

void Cluster::update(float dt)
{
    renderer->render(dt);

    if(renderer->buffers)
    {
        for(size_t i = 0; renderer->buffers[i].name != nullptr; i++)
        {
            //buffers[i].handle = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, bgfx::TextureFormat::RGBA8);

            //bgfx::updateTexture2D(buffers[i].handle, 0, 0, 0, 0, )
        }
    }

    ui.update(dt);
}

int Cluster::shutdown()
{
    renderer->shutdown();
    return 0;
}

void Cluster::BgfxCallbacks::screenShot(const char* name,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t pitch,
                                       const void* data,
                                       uint32_t /*size*/,
                                       bool yflip)
{
    // save screen shot as PNG
    char filePath[1024];
    bx::snprintf(filePath, BX_COUNTOF(filePath), "screenshots/%s.png", name);

    bx::Error err;
    // create directory if necessary
    bx::makeAll("screenshots", &err);
    if(err.isOk())
    {
        bx::FileWriter writer;
        if(bx::open(&writer, filePath, false, &err))
        {
            // write uncompressed PNG
            bimg::imageWritePng(&writer, width, height, pitch, data, bimg::TextureFormat::BGRA8, yflip, &err);
            bx::close(&writer);
        }
    }
}
