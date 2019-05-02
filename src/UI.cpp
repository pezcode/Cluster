#include "UI.h"

#include "Cluster.h"
#include "Config.h"
#include "Renderer.h"
#include <bx/string.h>
#include <IconsForkAwesome.h>

ClusterUI::ClusterUI(Cluster& app) :
    app(app),
    mTime(0.0f)
{
}

ClusterUI::~ClusterUI()
{
}

void ClusterUI::initialize()
{
    ImGuiIO& io = ImGui::GetIO();
    //io.IniFilename = nullptr;   // don't save window positions etc. to ini
    io.MouseDrawCursor = true; // let imgui draw cursors

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);

    // no round corners
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;

    // align text to the left
    // looks better with icon buttons
    style.ButtonTextAlign = ImVec2(0.0f, 0.5f);

    // Load text font
    io.Fonts->Clear();

    const char* fontFile = "assets/fonts/Roboto/Roboto-Medium.ttf";
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontFile, 14.0f);
    if(!font)
        io.Fonts->AddFontDefault();

    // Load and merge icon font

    const char* iconFontFile = "assets/fonts/ForkAwesome/forkawesome-webfont.ttf";
    const ImWchar iconsRanges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    ImFont* iconFont = io.Fonts->AddFontFromFileTTF(iconFontFile, 13.0f, &iconsConfig, iconsRanges);

    // Generate font texture

    unsigned char* tex_data;
    int tex_w, tex_h;
    int bytes;
    io.Fonts->GetTexDataAsRGBA32(&tex_data, &tex_w, &tex_h, &bytes);
    bgfx::TextureHandle texture = bgfx::createTexture2D((uint16_t)tex_w,
                                                        (uint16_t)tex_h,
                                                        false,
                                                        1,
                                                        bgfx::TextureFormat::RGBA8,
                                                        0,
                                                        bgfx::copy(tex_data, tex_w * tex_h * bytes));
    io.Fonts->SetTexID(ImTextureID(texture.idx));
}

void ClusterUI::update(float dt)
{
    mTime += dt;
    if(!app.config->showUI)
        return;

    const Renderer::TextureBuffer* buffers = app.renderer->buffers;

    // test
    Renderer::TextureBuffer temp[2] = { { bgfx::getTexture(app.renderer->frameBuffer), "Output" }, { 0, nullptr } };
    buffers = temp;

    //ImGui::ShowDemoWindow();

    ImVec2 padding = { 5.0f, 5.0f };

    if(app.config->showConfigWindow)
    {
        ImGui::Begin("Settings", &app.config->showConfigWindow, ImGuiWindowFlags_AlwaysAutoResize);

        // Render path radio buttons
        ImGui::Text("Render path:");
        static int renderPathSelected = app.config->renderPath;
        ImGui::RadioButton("Forward", &renderPathSelected, Cluster::Forward);
        ImGui::RadioButton("Deferred", &renderPathSelected, Cluster::Deferred);
        ImGui::RadioButton("Clustered", &renderPathSelected, Cluster::Clustered);
        Cluster::RenderPath path = Cluster::RenderPath(renderPathSelected);
        if(path != app.config->renderPath)
            app.setRenderPath(path);

        ImGui::Separator();
        ImGui::Checkbox("Show log", &app.config->showLog);
        ImGui::Checkbox("Show stats", &app.config->showStatsOverlay);
        if(buffers)
            ImGui::Checkbox("Show buffers", &app.config->showBuffers);
        if(ImGui::Button(ICON_FK_CAMERA "  Screenshot", ImVec2(100, 0)))
        {
            static unsigned int count = 0;
            count++;
            char name[32];// = "screenshots/";
            bx::toString(name, BX_COUNTOF(name), count);
            app.saveFrameBuffer(app.renderer->frameBuffer, name);
            // this takes a screenshot of the OS window framebuffer, UI included
            // bgfx::requestScreenShot(BGFX_INVALID_HANDLE, name);
        }
        if(ImGui::Button(app.config->fullscreen ? (ICON_FK_WINDOW_RESTORE "  Restore")
                                                : (ICON_FK_WINDOW_MAXIMIZE "  Fullscreen"),
                         ImVec2(100, 0)))
            app.toggleFullscreen();
        if(ImGui::Button(ICON_FK_EYE_SLASH "  Hide UI", ImVec2(100, 0)))
            app.config->showUI = false;
        ImGui::SameLine();
        // disabled look
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.4f);
        ImGui::Text("%s to restore", glfwGetKeyName(GLFW_KEY_R, 0));
        ImGui::PopStyleVar();
        ImGui::End();
    }

    // log
    if(app.config->showLog)
    {
        ImGui::Begin("Log", &app.config->showLog, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(app.log.getPtr());
        // only scroll down if it's currently at the bottom
        if(ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::End();
    }

    // performance overlay
    if(app.config->showStatsOverlay)
    {
        // top left, transparent background
        ImGui::SetNextWindowPos(padding, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Stats",
                     nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoNav);

        // title
        ImGui::Text(ICON_FK_TACHOMETER " Stats");
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.4f);
        ImGui::Text("right-click to toggle");
        ImGui::PopStyleVar();
        ImGui::Separator();

        // general data
        ImGui::Text("Backend: %s", bgfx::getRendererName(bgfx::getRendererType()));
        const bgfx::Stats* stats = bgfx::getStats();
        ImGui::Text("Buffer size: %u x %u px", stats->width, stats->height);
        ImGui::Text("Triangles: %u", stats->numPrims[bgfx::Topology::TriList]);
        ImGui::Text("Draw calls: %u", stats->numDraw);
        ImGui::Text("Compute calls: %u", stats->numCompute);

        // plots
        static float fpsValues[GRAPH_HISTORY] = { 0 };
        static float frameTimeValues[GRAPH_HISTORY] = { 0 };
        static float gpuMemoryValues[GRAPH_HISTORY] = { 0 };
        static int offset = 0;

        if(app.config->overlays.fps)
        {
            ImGui::Separator();
            ImGui::Text("FPS");
            ImGui::PlotLines(
                "", fpsValues, IM_ARRAYSIZE(fpsValues), offset + 1, nullptr, 0.0f, 200.0f, ImVec2(150, 50));
            ImGui::Text("%.0f", fpsValues[offset]);
        }
        if(app.config->overlays.frameTime)
        {
            ImGui::Separator();
            ImGui::Text("Frame time");
            ImGui::PlotLines(
                "", frameTimeValues, IM_ARRAYSIZE(frameTimeValues), offset + 1, nullptr, 0.0f, 30.0f, ImVec2(150, 50));
            ImGui::Text("CPU: %.2f ms", float(stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000.0f / stats->cpuTimerFreq);
            ImGui::Text("GPU: %.2f ms", float(stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000.0f / stats->gpuTimerFreq);
            ImGui::Text("Total: %.2f", frameTimeValues[offset]);
        }
        if(app.config->overlays.gpuMemory)
        {
            float used = float(stats->gpuMemoryUsed) / 1000 / 1000;
            float max = float(stats->gpuMemoryMax) / 1000 / 1000;

            ImGui::Separator();
            if(used > 0.0f && max > 0.0f)
            {
                ImGui::Text("GPU memory");
                ImGui::PlotLines(
                    "", gpuMemoryValues, IM_ARRAYSIZE(gpuMemoryValues), offset + 1, nullptr, 0.0f, max, ImVec2(150, 50));
                ImGui::Text("%.0f / %.0f MB", used, max);
            }
            else
            {
                ImGui::TextWrapped(ICON_FK_EXCLAMATION_TRIANGLE " GPU memory data unavailable");
            }
        }

        // update after drawing so offset is the current value
        static float oldTime = 0.0f;
        if(mTime - oldTime > GRAPH_FREQUENCY)
        {
            offset = (offset + 1) % GRAPH_HISTORY;
            ImGuiIO& io = ImGui::GetIO();
            fpsValues[offset] = 1 / io.DeltaTime;
            frameTimeValues[offset] = io.DeltaTime * 1000;
            gpuMemoryValues[offset] = float(stats->gpuMemoryUsed) / 1000 / 1000;

            oldTime = mTime;
        }

        // right click menu
        if(ImGui::BeginPopupContextWindow())
        {
            ImGui::Checkbox("FPS", &app.config->overlays.fps);
            ImGui::Checkbox("Frame time", &app.config->overlays.frameTime);
            ImGui::Checkbox("GPU memory", &app.config->overlays.gpuMemory);
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    if(buffers && app.config->showBuffers)
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

void ClusterUI::shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    bgfx::TextureHandle fontTexture = BGFX_INVALID_HANDLE;
    io.Fonts->SetTexID(ImTextureID(uintptr_t(fontTexture.idx)));
    fontTexture.idx = uint16_t(uintptr_t(io.Fonts->TexID));
    bgfx::destroy(fontTexture);
}

void ClusterUI::imageTooltip(ImTextureID tex, ImVec2 tex_size, float region_size)
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
