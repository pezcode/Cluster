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

int Cluster::run(int argc, char* argv[])
{
    config.readArgv(argc, argv);
    return Application::run(argc, argv, bgfx::RendererType::Count, BGFX_PCI_ID_NONE, 0, &callbacks, nullptr);
}

void Cluster::initialize(int _argc, char* _argv[])
{
    // TODO read from config
    reset(BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X8 | BGFX_RESET_MAXANISOTROPY);
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    // TODO load app icon
    // not working
    // https://github.com/bkaradzic/bgfx/blob/4cd8e574e826dc3b6998790052f7aefac6cc1164/examples/common/bgfx_utils.cpp#L168

    /*
    bx::Error err;
    bx::FileReader reader;
    if(bx::open(&reader, "assets/icons/cube.dds", &err))
    {
        bx::DefaultAllocator allocator;

        uint32_t size = (uint32_t)bx::getSize(&reader);
        void* data = BX_ALLOC(&allocator, size);
        bx::read(&reader, data, size);
        bx::close(&reader);

        bimg::ImageContainer image;
        if(bimg::imageParse(image, data, size, &err))
        {
            bimg::ImageContainer* converted = bimg::imageConvert(&allocator, bimg::TextureFormat::RGBA8, image, false);
            if(converted)
            {
                if(converted->m_data)
                {
                    GLFWimage icon;
                    icon.width = converted->m_width;
                    icon.height = converted->m_height;
                    icon.pixels = (unsigned char*)converted->m_data;
                
                    glfwSetWindowIcon(nullptr, 1, &icon);
                }
                bimg::imageFree(converted);
            }
        }

        BX_FREE(&allocator, data);
    }
    */

    renderer->initialize();
    ui.initialize();
}

void Cluster::onReset()
{
    renderer->reset(uint16_t(getWidth()), uint16_t(getHeight()));
}

void Cluster::onKey(int key, int scancode, int action, int mods)
{
    if(action == GLFW_RELEASE)
    {
        switch(key)
        {
            case GLFW_KEY_R:
                config.showUI = true;
                config.showConfigWindow = true;
                break;
        }
    }
}

void Cluster::update(float dt)
{
    if(mFrameNumber > 0 && mFrameNumber == saveFrame)
    {
        // TODO create thread
        callbacks.screenShot("hehe", getWidth(), getHeight(), 0, saveData, 0, false);
        saveFrame = 0;
        BX_FREE(&allocator, saveData);
        saveData = nullptr;
    }
    renderer->render(dt);
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

void Cluster::saveFrameBuffer(bgfx::FrameBufferHandle frameBuffer, const char* name)
{
    if(saveData)
        return;

    if(bgfx::getCaps()->supported & BGFX_CAPS_TEXTURE_READ_BACK)
    {
        bgfx::TextureHandle texture = bgfx::getTexture(frameBuffer);
        if(isValid(texture))
        {
            bgfx::TextureInfo info;
            bgfx::calcTextureSize(info, getWidth(), getHeight(), 1, false, false, 1, bgfx::TextureFormat::BGRA8);
            saveData = BX_ALLOC(&allocator, info.storageSize);
            saveFrame = bgfx::readTexture(texture, saveData);
        }
    }
}
