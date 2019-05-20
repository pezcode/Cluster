#include "Cluster.h"

#include "UI.h"
#include "Config.h"
#include "Scene/Scene.h"
#include "Log/Log.h"
#include "Renderer/ForwardRenderer.h"
#include "Renderer/DeferredRenderer.h"
#include "Renderer/ClusteredRenderer.h"
#include <bx/file.h>
#include <bx/string.h>
#include <bimg/bimg.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <thread>
#include <algorithm> 

bx::DefaultAllocator Cluster::allocator;
bx::AllocatorI* Cluster::iAlloc = &allocator;

Cluster::Cluster() :
    bigg::Application("Cluster", 1024, 768),
    //deltaTime(0.0f),
    mouseX(-1.0f),
    mouseY(-1.0f),
    saveFrame(0),
    saveData(nullptr),
    callbacks(*this),
    config(std::make_unique<Config>()),
    ui(std::make_unique<ClusterUI>(*this)),
    scene(std::make_unique<Scene>()),
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
    // is _mt (thread safe) necessary?
    spdlog::sink_ptr fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config->logFile, true);
    fileSink->set_level(spdlog::level::trace);
    fileSink->set_pattern("[%H:%M:%S][%l] %v");
    Sinks->add_sink(fileSink);

    Log->flush_on(spdlog::level::info);
    Log->set_level(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(2));
    // TODO remove sink during shutdown

    uint32_t resetFlags = BGFX_RESET_MAXANISOTROPY;
    if(config->vsync)
        resetFlags |= BGFX_RESET_VSYNC;
    reset(resetFlags);

    if(config->fullscreen)
        toggleFullscreen();

    renderer->initialize();
    ui->initialize();

    if(!ForwardRenderer::supported())
        Log->warn("Forward renderer not supported on this hardware");
    if(!DeferredRenderer::supported())
        Log->warn("Deferred renderer not supported on this hardware");
    if(!ClusteredRenderer::supported())
        Log->warn("Clustered renderer not supported on this hardware");

    Scene::init();
    // TODO multithreaded
    // textures still have to be loaded from main thread
    // keep list and load one texture every X frames
    if(!scene->load(config->sceneFile))
    {
        Log->error("Loading scene model failed");
        close();
        return;
    }
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

    // camera input handling
    // GLFW only sends repeat event for the last key pressed
    // need to keep track of which keys are currently down
    // TODO put in separate class
    switch(key)
    {
        case GLFW_KEY_W:
        case GLFW_KEY_A:
        case GLFW_KEY_S:
        case GLFW_KEY_D:
        case GLFW_KEY_SPACE:
        case GLFW_KEY_LEFT_CONTROL:
            keys[key] = action != GLFW_RELEASE;
            break;
    }
}

void Cluster::onCursorPos(double xpos, double ypos)
{
    constexpr float angularVelocity = 180.0f / 600.0f; // degrees/pixel

    if(mouseX >= 0.0f && mouseY >= 0.0f)
    {   
        if(glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            scene->camera.rotate(glm::vec3(-(ypos - mouseY), -(xpos - mouseX), 0.0f) * angularVelocity);
        }
    }
    mouseX = xpos;
    mouseY = ypos;
}

void Cluster::onCursorEnter(int entered)
{
    if(!entered) // lost focus
    {
        mouseX = mouseY = -1.0f;
    }
}

void Cluster::onScroll(double xoffset, double yoffset)
{
    // wheel scrolled up = zoom in by 2 extra degrees
    scene->camera.zoom((float)yoffset * 2.0f);
}

void Cluster::update(float dt)
{
    //deltaTime = dt;

    constexpr float velocity = 3.0f; // m/s
    if(keys[GLFW_KEY_W])
        scene->camera.move(scene->camera.forward() * velocity * dt);
    if(keys[GLFW_KEY_A])
        scene->camera.move(-scene->camera.right() * velocity * dt);
    if(keys[GLFW_KEY_S])
        scene->camera.move(-scene->camera.forward() * velocity * dt);
    if(keys[GLFW_KEY_D])
        scene->camera.move(scene->camera.right() * velocity * dt);
    if(keys[GLFW_KEY_SPACE])
        scene->camera.move(scene->camera.up() * velocity * dt);
    if(keys[GLFW_KEY_LEFT_CONTROL])
        scene->camera.move(-scene->camera.up() * velocity * dt);

    // screenshot texture data is ready, save in a new thread
    if(mFrameNumber > 0 && mFrameNumber == saveFrame)
    {
        // async destructor blocks, even if not assigned
        // thread destructor also blocks, unless detached
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
    // TODO
    // not all resources are freed
    // e.g. Renderer::blitSampler has count 3 on shutdown
    // might be because of threaded renderer or command buffer taking a while

    ui->shutdown();
    renderer->shutdown();
    scene->clear();
    return 0;
}

void Cluster::BgfxCallbacks::fatal(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* str)
{
    if(code != bgfx::Fatal::DebugCheck)
    {
        // unrecoverable, terminate
        // don't log debug checks either, their output gets sent to traceVargs
        Log->critical("{}", str);
        app.close();
    }
}

// only called when compiled as debug
void Cluster::BgfxCallbacks::traceVargs(const char* filePath, uint16_t line, const char* format, va_list args)
{
    char buffer[1024];
    int32_t written = bx::vsnprintf(buffer, BX_COUNTOF(buffer), format, args);
    if(written > 0 && written < BX_COUNTOF(buffer))
    {
        // bgfx sends lines with newlines, spdlog adds another
        if(buffer[written-1] == '\n')
            buffer[written - 1] = '\0';
        Log->trace(buffer);
    }
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
            if(!ForwardRenderer::supported())
                Log->error("Forward renderer not supported");
            else
                renderer = std::make_unique<ForwardRenderer>(scene.get());
            break;
        case Deferred:
            if(!DeferredRenderer::supported())
                Log->error("Deferred renderer not supported");
            else
                renderer = std::make_unique<DeferredRenderer>(scene.get());
            break;
        case Clustered:
            if(!ClusteredRenderer::supported())
                Log->error("Clustered renderer not supported");
            else
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
