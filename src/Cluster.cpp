#include "Cluster.h"

#include "UI.h"
#include "Config.h"
#include "Scene.h"

#include "ForwardRenderer.h"
#include "DeferredRenderer.h"
#include "ClusteredRenderer.h"

#include <bx/file.h>
#include <bx/string.h>
#include <bimg/bimg.h>

#include <thread>

bx::DefaultAllocator Cluster::allocator;
bx::AllocatorI* Cluster::iAlloc = &allocator;

Cluster::Cluster() :
    bigg::Application("Cluster", 1024, 768),
    config(std::make_unique<Config>()),
    ui(std::make_unique<ClusterUI>(*this)),
    scene(std::make_unique<Scene>()),
    callbacks(*this),
    renderer(nullptr)
{
}

Cluster::~Cluster()
{
    // we need to define the destructor where the forward-declared classes are actually defined by now
    // otherwise we get a default constructor defined in the header
    // then unique_ptr can't delete the object and compilation fails
}

int Cluster::run(int argc, char* argv[])
{
    config->readArgv(argc, argv);
    return Application::run(argc, argv, config->renderer, BGFX_PCI_ID_NONE, 0, &callbacks, nullptr);
}

void Cluster::initialize(int _argc, char* _argv[])
{
    // TODO read from config
    //reset(BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X8 | BGFX_RESET_MAXANISOTROPY);
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

    if(config->fullscreen)
        toggleFullscreen();

    renderer->initialize();
    ui->initialize();
}

void Cluster::onReset()
{
    // init renderer here
    // onReset is called before initialize on startup
    if(!renderer)
        setRenderPath(config->renderPath);

    renderer->reset(uint16_t(getWidth()), uint16_t(getHeight()));
}

void Cluster::onKey(int key, int scancode, int action, int mods)
{
    if(action == GLFW_RELEASE)
    {
        switch(key)
        {
            case GLFW_KEY_ESCAPE:
                close();
                break;
            case GLFW_KEY_R:
                config->showUI = true;
                config->showConfigWindow = true;
                break;
        }
    }
}

void Cluster::update(float dt)
{
    // screenshot texture data is ready, save in a new thread
    if(mFrameNumber > 0 && mFrameNumber == saveFrame)
    {
        // async destructor blocks, even if not assigned
        // thread also blocks, unless detached
        std::thread([this]() {
            callbacks.screenShot("hehe", getWidth(), getHeight(), 0, saveData, 0, false);
            saveFrame = 0;
            BX_FREE(&allocator, saveData);
            saveData = nullptr;
        }).detach();
    }
    renderer->render(dt);
    ui->update(dt);
}

int Cluster::shutdown()
{
    ui->shutdown();
    renderer->shutdown();
    return 0;
}

void Cluster::BgfxCallbacks::fatal(const char* _filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char* _str)
{

}

void Cluster::BgfxCallbacks::traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList)
{
    char out[1024+2];
    bx::vsnprintf(out, BX_COUNTOF(out)-2, _format, _argList);
    //bx::strCat(out, BX_COUNTOF(out), "\n");
    app.log.append(out);
    //app.log.append("\n");
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

void Cluster::close()
{
    glfwSetWindowShouldClose(mWindow, 1);
}

void Cluster::toggleFullscreen()
{
    static int oldX = 0, oldY = 0;
    static int oldWidth = 0, oldHeight = 0;

    // GLFW didn't create the context, bgfx did
    // glfwGetCurrentContext returns null
    GLFWwindow* window = mWindow; //glfwGetCurrentContext();
    if(glfwGetWindowMonitor(window))
    {
        glfwSetWindowMonitor(window, NULL, oldX, oldY, oldWidth, oldHeight, 0);
        config->fullscreen = false;
    }
    else
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if(NULL != monitor)
        {
            glfwGetWindowPos(window, &oldX, &oldY);
            glfwGetWindowSize(window, &oldWidth, &oldHeight);

            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            config->fullscreen = true;
        }
    }
}

void Cluster::setRenderPath(RenderPath path)
{
    if(renderer && path == config->renderPath)
        return;

    if(renderer)
        renderer->shutdown();

    renderer.release();
    switch(path)
    {
        case Forward:
            renderer = std::make_unique<ForwardRenderer>(scene.get());
            break;
        case Deferred:
            renderer = std::make_unique<DeferredRenderer>(scene.get());
            break;
        case Clustered:
            renderer = std::make_unique<ClusteredRenderer>(scene.get());
            break;
        default:
            renderer = nullptr;
            break;
    }

    // TODO error out if no renderer

    if(renderer)
    {
        renderer->reset(getWidth(), getHeight());
        renderer->initialize();
    }

    config->renderPath = path;
}

void Cluster::saveFrameBuffer(bgfx::FrameBufferHandle frameBuffer, const char* name)
{
    if(saveData)
        return;

    if((bgfx::getCaps()->supported & BGFX_CAPS_TEXTURE_READ_BACK) == BGFX_CAPS_TEXTURE_READ_BACK)
    {
        // this isn't working for frame buffers (textures with render target flag)
        // TODO blit to separate texture and then read that
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
