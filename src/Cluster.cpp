#include "Cluster.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

struct PosColorVertex
{
    float x;
    float y;
    float z;
    uint32_t abgr;
    static void init()
    {
        ms_decl.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }
    static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[] = {
    { -1.0f, 1.0f, 1.0f, 0xff000000 },   { 1.0f, 1.0f, 1.0f, 0xff0000ff },   { -1.0f, -1.0f, 1.0f, 0xff00ff00 },
    { 1.0f, -1.0f, 1.0f, 0xff00ffff },   { -1.0f, 1.0f, -1.0f, 0xffff0000 }, { 1.0f, 1.0f, -1.0f, 0xffff00ff },
    { -1.0f, -1.0f, -1.0f, 0xffffff00 }, { 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t s_cubeTriList[] = { 2, 1, 0, 2, 3, 1, 5, 6, 4, 7, 6, 5, 4, 2, 0, 6, 2, 4,
                                          3, 5, 1, 3, 7, 5, 1, 4, 0, 1, 5, 4, 6, 3, 2, 7, 3, 6 };

Cluster::Cluster() :
    bigg::Application("Cluster", 1024, 768),
    ui(config, buffers),
    buffers{ { 0, "Albedo" }, { 0, "Normal" }, { 0, "Specular" }, { 0, nullptr } }
{
}

void Cluster::initialize(int _argc, char* _argv[])
{
    PosColorVertex::init();

    char vsName[32];
    char fsName[32];

    const char* shaderPath = "???";

    switch(bgfx::getRendererType())
    {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D9:
            shaderPath = "shaders/dx9/";
            break;
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            shaderPath = "shaders/dx11/";
            break;
        case bgfx::RendererType::Gnm:
            break;
        case bgfx::RendererType::Metal:
            shaderPath = "shaders/metal/";
            break;
        case bgfx::RendererType::OpenGL:
            shaderPath = "shaders/glsl/";
            break;
        case bgfx::RendererType::OpenGLES:
            shaderPath = "shaders/essl/";
            break;
        case bgfx::RendererType::Vulkan:
            break;
        case bgfx::RendererType::Count:
            break;
    }

    bx::strCopy(vsName, BX_COUNTOF(vsName), shaderPath);
    bx::strCat(vsName, BX_COUNTOF(vsName), "vs_cubes.bin");

    bx::strCopy(fsName, BX_COUNTOF(fsName), shaderPath);
    bx::strCat(fsName, BX_COUNTOF(fsName), "fs_cubes.bin");

    mProgram = bigg::loadProgram(vsName, fsName);
    mVbh = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_decl);
    mIbh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));
    bgfx::setDebug(BGFX_DEBUG_TEXT);
    mTime = 0.0f;

    for(size_t i = 0; buffers[i].name != nullptr; i++)
    {
        buffers[i].handle = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, bgfx::TextureFormat::RGBA8);
    }

    ui.initialize();
}

void Cluster::onReset()
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x30303000, 1.0f, 0);
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
    mTime += dt;
    glm::mat4 view =
        glm::lookAt(glm::vec3(0.0f, 0.0f, -35.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = bigg::perspective(glm::radians(60.0f), float(getWidth()) / getHeight(), 0.1f, 100.0f);
    bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
    bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t(getHeight()));
    //bgfx::touch(0);

    for(uint32_t yy = 0; yy < 11; ++yy)
    {
        for(uint32_t xx = 0; xx < 11; ++xx)
        {
            glm::mat4 mtx;
            mtx = glm::translate(mtx, glm::vec3(15.0f - float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 0.0f));
            mtx *= glm::yawPitchRoll(mTime + xx * 0.21f, mTime + yy * 0.37f, 0.0f);
            bgfx::setTransform(&mtx[0][0]);
            bgfx::setVertexBuffer(0, mVbh);
            bgfx::setIndexBuffer(mIbh);
            bgfx::setState(BGFX_STATE_DEFAULT);
            bgfx::submit(0, mProgram);
        }
    }

    //bgfx::TextureHandle frameBuffer = bgfx::getTexture();

    for(size_t i = 0; buffers[i].name != nullptr; i++)
    {
        //buffers[i].handle = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, bgfx::TextureFormat::RGBA8);

        //bgfx::updateTexture2D(buffers[i].handle, 0, 0, 0, 0, )
    }

    ui.update(mTime);
}

void Cluster::UI::initialize()
{
    ImGuiIO& io = ImGui::GetIO();
    //io.IniFilename = nullptr;   // don't save window positions etc. to ini
    io.MouseDrawCursor = true; // let imgui draw cursors

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;

    // Load font

    io.Fonts->Clear();
    const char* fontFile = "assets/fonts/Roboto/Roboto-Medium.ttf";
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontFile, 14.0f);
    if(!font)
        io.Fonts->AddFontDefault();

    // Generate font texture

    unsigned char* tex_data;
    int tex_w, tex_h;
    int bytes;
    io.Fonts->GetTexDataAsRGBA32(&tex_data, &tex_w, &tex_h, &bytes);
    bgfx::TextureHandle tex = bgfx::createTexture2D((uint16_t)tex_w,
                                                    (uint16_t)tex_h,
                                                    false,
                                                    1,
                                                    bgfx::TextureFormat::RGBA8,
                                                    0,
                                                    bgfx::copy(tex_data, tex_w * tex_h * bytes));
    io.Fonts->SetTexID(ImTextureID(tex.idx));
}

void Cluster::UI::update(float time)
{
    if(!config.showUI)
        return;

    //ImGui::ShowDemoWindow();

    ImVec2 padding = { 5.0f, 5.0f };

    if(config.showConfigWindow)
    {
        ImGui::Begin("Settings", &config.showConfigWindow, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Checkbox("Performance overlay", &config.showPerfOverlay);
        ImGui::Checkbox("Buffers", &config.showBuffers);
        if(ImGui::Button("Hide UI"))
            config.showUI = false;
        ImGui::SameLine();
        // disabled look
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Text("(R to restore)");
        ImGui::PopStyleVar();
        ImGui::End();
    }

    // performance overlay
    if(config.showPerfOverlay)
    {
        // top left, transparent background
        ImGui::SetNextWindowPos(padding, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Performance",
                     nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoNav);

        // title
        ImGui::Text("Performance");
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Text("(right-click to toggle)");
        ImGui::PopStyleVar();
        ImGui::Separator();

        // general data
        const bgfx::Stats* stats = bgfx::getStats();
        ImGui::Text("Backbuffer: %u x %u", stats->width, stats->height);
        ImGui::Text("Triangles: %u", stats->numPrims[bgfx::Topology::TriList]);
        ImGui::Text("Draw calls: %u", stats->numDraw);
        ImGui::Text("Compute calls: %u", stats->numCompute);

        // plots
        constexpr size_t history = 100;
        static float fpsValues[history] = { 0 };
        static float frameTimeValues[history] = { 0 };
        static float gpuMemoryValues[history] = { 0 };
        static size_t offset = 0;

        if(config.overlays.fps)
        {
            ImGui::Separator();
            ImGui::Text("FPS");
            ImGui::PlotLines(
                "", fpsValues, IM_ARRAYSIZE(fpsValues), offset + 1, nullptr, 0.0f, 200.0f, ImVec2(150, 50));
            ImGui::Text("%.0f", fpsValues[offset]);
        }
        if(config.overlays.frameTime)
        {
            ImGui::Separator();
            ImGui::Text("Frame time");
            ImGui::PlotLines(
                "", frameTimeValues, IM_ARRAYSIZE(frameTimeValues), offset + 1, nullptr, 0.0f, 30.0f, ImVec2(150, 50));
            ImGui::Text("CPU: %.2f ms", float(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0f / stats->cpuTimerFreq);
            ImGui::Text("GPU: %.2f ms", float(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0f / stats->gpuTimerFreq);
            ImGui::Text("Total: %.2f", frameTimeValues[offset]);
        }
        if(config.overlays.gpuMemory)
        {
            float used = stats->gpuMemoryUsed / 1000 / 1000;
            float max = stats->gpuMemoryMax / 1000 / 1000;

            ImGui::Separator();
            ImGui::Text("GPU memory");
            ImGui::PlotLines(
                "", gpuMemoryValues, IM_ARRAYSIZE(gpuMemoryValues), offset + 1, nullptr, 0.0f, max, ImVec2(150, 50));
            ImGui::Text("%.0f / %.0f MB", used, max);
        }

        // update after drawing so offset is the current value
        static float oldTime = 0.0f;
        if(time - oldTime > 0.1f)
        {
            // update 10 times per second
            offset = (offset + 1) % history;
            ImGuiIO& io = ImGui::GetIO();
            fpsValues[offset] = 1 / io.DeltaTime;
            frameTimeValues[offset] = io.DeltaTime * 1000;
            gpuMemoryValues[offset] = stats->gpuMemoryUsed / 1000 / 1000;

            oldTime = time;
        }

        // right click menu
        if(ImGui::BeginPopupContextWindow())
        {
            ImGui::Checkbox("FPS", &config.overlays.fps);
            ImGui::Checkbox("Frame time", &config.overlays.frameTime);
            ImGui::Checkbox("GPU memory", &config.overlays.gpuMemory);
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    if(config.showBuffers)
    {
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Buffers",
                     nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoNav);

        ImGuiIO& io = ImGui::GetIO();

        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);

        for(size_t i = 0; buffers[i].name != nullptr; i++)
        {
            ImGui::Text(buffers[i].name);
            ImTextureID tex_id = ImTextureID(uintptr_t(buffers[i].handle.idx));
            ImVec2 tex_size = io.DisplaySize;
            tex_size = { 128.0f, 128.0f };
            ImGui::Image(tex_id, tex_size, ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
            imageTooltip(tex_id, tex_size, 32);
        }

        // move window to bottom right
        ImVec2 display_size = io.DisplaySize;
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 window_pos(display_size.x - window_size.x - padding.x, display_size.y - window_size.y - padding.y);
        ImGui::SetWindowPos(window_pos);

        ImGui::End();
    }
}

void Cluster::UI::imageTooltip(ImTextureID tex, ImVec2 tex_size, float region_size)
{
    ImGuiIO& io = ImGui::GetIO();
    if(ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float region_x = io.MousePos.x - pos.x - region_size * 0.5f;
        if(region_x < 0.0f)
            region_x = 0.0f;
        else if(region_x > tex_size.x - region_size)
            region_x = tex_size.x - region_size;
        float region_y = io.MousePos.y - pos.y - region_size * 0.5f;
        if(region_y < 0.0f)
            region_y = 0.0f;
        else if(region_y > tex_size.y - region_size)
            region_y = tex_size.y - region_size;
        float zoom = 4.0f;
        ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
        ImGui::Text("Max: (%.2f, %.2f)", region_x + region_size, region_y + region_size);
        ImVec2 uv0 = ImVec2((region_x) / tex_size.x, (region_y) / tex_size.y);
        ImVec2 uv1 = ImVec2((region_x + region_size) / tex_size.x, (region_y + region_size) / tex_size.y);
        ImGui::Image(tex,
                     ImVec2(region_size * zoom, region_size * zoom),
                     uv0,
                     uv1,
                     ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                     ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        ImGui::EndTooltip();
    }
}
