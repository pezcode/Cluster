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
#include <glm/gtx/component_wise.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <algorithm>
#include <random>

Cluster::Cluster() :
    bigg::Application("Cluster", 1280, 720),
    callbacks(*this),
    config(std::make_unique<Config>()),
    ui(std::make_unique<ClusterUI>(*this)),
    scene(std::make_unique<Scene>())
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
    if(config->writeLog)
    {
        // _mt (thread safe) necessary because of flush_every
        logFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config->logFile, true);
        logFileSink->set_level(spdlog::level::trace);
        logFileSink->set_pattern("[%H:%M:%S][%l] %v");
        Sinks->add_sink(logFileSink);
    }

    Log->flush_on(spdlog::level::info);
    Log->set_level(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(2));

    if(!ForwardRenderer::supported())
    {
        Log->error("Forward renderer not supported on this hardware");
        close();
        return;
    }
    if(!DeferredRenderer::supported())
    {
        Log->error("Deferred renderer not supported on this hardware");
        close();
        return;
    }
    if(!ClusteredRenderer::supported())
    {
        Log->error("Clustered renderer not supported on this hardware");
        close();
        return;
    }

    if(config->profile)
        bgfx::setDebug(BGFX_DEBUG_PROFILER);

    uint32_t resetFlags = BGFX_RESET_MAXANISOTROPY | BGFX_RESET_SRGB_BACKBUFFER;
    if(config->vsync)
        resetFlags |= BGFX_RESET_VSYNC;
    reset(resetFlags);

    if(config->fullscreen)
        toggleFullscreen();

    if(glfwRawMouseMotionSupported())
        glfwSetInputMode(mWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // renderer has already been created in onReset
    renderer->setTonemappingMode(config->tonemappingMode);
    renderer->setMultipleScattering(config->multipleScattering);
    ui->initialize();

    Scene::init();

    if(!scene->load(config->sceneFile))
    {
        Log->error("Loading scene model failed");
        close();
        return;
    }

    // Sponza debug camera + lights
    if(!config->customScene)
    {
        scene->camera.lookAt({ -7.0f, 2.0f, 0.0f }, scene->center, glm::vec3(0.0f, 1.0f, 0.0f));
        scene->pointLights.lights = { // pos, power
                                      { { -5.0f, 0.3f, 0.0f }, { 100.0f, 100.0f, 100.0f } },
                                      { { 0.0f, 0.3f, 0.0f }, { 100.0f, 100.0f, 100.0f } },
                                      { { 5.0f, 0.3f, 0.0f }, { 100.0f, 100.0f, 100.0f } }
        };
    }

    scene->pointLights.update();
    config->lights = (int)scene->pointLights.lights.size();
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

void Cluster::onCursorPos(double xpos, double ypos)
{
    constexpr float angularVelocity = 180.0f / 600.0f; // degrees/pixel

    if(mouseX >= 0.0f && mouseY >= 0.0f)
    {
        if(isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
        {
            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            scene->camera.rotate(-glm::vec2(ypos - mouseY, xpos - mouseX) * angularVelocity);
        }
        else
        {
            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
    const float t = (float)glfwGetTime();

    float velocity = scene->diagonal / 5.0f; // m/s
    if(isKeyDown(GLFW_KEY_W))
        scene->camera.move(scene->camera.forward() * velocity * dt);
    if(isKeyDown(GLFW_KEY_A))
        scene->camera.move(-scene->camera.right() * velocity * dt);
    if(isKeyDown(GLFW_KEY_S))
        scene->camera.move(-scene->camera.forward() * velocity * dt);
    if(isKeyDown(GLFW_KEY_D))
        scene->camera.move(scene->camera.right() * velocity * dt);
    if(isKeyDown(GLFW_KEY_SPACE))
        scene->camera.move(scene->camera.up() * velocity * dt);
    if(isKeyDown(GLFW_KEY_LEFT_CONTROL))
        scene->camera.move(-scene->camera.up() * velocity * dt);

    if(config->movingLights)
        moveLights(t, dt);
    scene->pointLights.update();

    renderer->render(dt);
    ui->update(dt);
}

int Cluster::shutdown()
{
    ui->shutdown();
    renderer->shutdown();
    scene->clear();
    Sinks->remove_sink(logFileSink);
    logFileSink = nullptr;
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
        if(buffer[written - 1] == '\n')
            buffer[written - 1] = '\0';
        Log->trace(buffer);
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
        case RenderPath::Forward:
            renderer = std::make_unique<ForwardRenderer>(scene.get());
            break;
        case RenderPath::Deferred:
            renderer = std::make_unique<DeferredRenderer>(scene.get());
            break;
        case RenderPath::Clustered:
            renderer = std::make_unique<ClusteredRenderer>(scene.get());
            break;
        default:
            assert(false);
            break;
    }

    renderer->reset(getWidth(), getHeight());
    renderer->initialize();

    config->renderPath = path;
}

void Cluster::generateLights(unsigned int count)
{
    // TODO? normalize power

    auto& lights = scene->pointLights.lights;

    size_t keep = lights.size();
    if(count < keep)
        keep = count;

    lights.resize(count);

    glm::vec3 scale = glm::abs(scene->maxBounds - scene->minBounds) * 0.75f;

    constexpr float POWER_MIN = 20.0f;
    constexpr float POWER_MAX = 100.0f;

    std::random_device rd;
    std::seed_seq seed = { rd() };
    std::mt19937 mt(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for(size_t i = keep; i < count; i++)
    {
        glm::vec3 position = scene->center;
        position += glm::vec3(dist(mt), dist(mt), dist(mt)) * scale - (scale * 0.5f);

        if(!config->customScene) // Sponza, no lights under the floor
            position.y = glm::abs(position.y);

        glm::vec3 color = glm::vec3(dist(mt), dist(mt), dist(mt));
        glm::vec3 power = color * (dist(mt) * (POWER_MAX - POWER_MIN) + POWER_MIN);
        lights[i] = { position, power };
    }
}

void Cluster::moveLights(float t, float dt)
{
    const float angularVelocity = glm::radians(10.0f);
    const float angle = angularVelocity * dt;
    //const glm::vec3 translationExtent = glm::abs(scene->maxBounds - scene->minBounds) * glm::vec3( 0.1f, 0.0f, 0.1f ); // { 1.0f, 0.0f, 1.0f };

    for(PointLight& light : scene->pointLights.lights)
    {
        light.position =
            glm::mat3(glm::rotate(glm::identity<glm::mat4>(), angle, glm::vec3(0.0f, 1.0f, 0.0f))) * light.position;
        //light.position += glm::sin(glm::vec3(t) * glm::vec3(1.0f, 2.0f, 3.0f)) * translationExtent * dt;
    }
}
