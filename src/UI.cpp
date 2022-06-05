#include "UI.h"

#include "Cluster.h"
#include "Scene/Scene.h"
#include "Config.h"
#include "Renderer/Renderer.h"
#include "Log/UISink.h"
#include "Log/Log.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/color_space.hpp>
#include <bx/string.h>
#include <IconsForkAwesome.h>
#include <functional>
#include <cctype>

using namespace std::placeholders;

ClusterUI::ClusterUI(Cluster& app) : app(app) { }

void ClusterUI::initialize()
{
    // Log

    auto func = std::bind(&ClusterUI::log, this, std::placeholders::_1, std::placeholders::_2);
    logUISink = std::make_shared<spdlog::ext::clusterui_sink_mt<decltype(func)>>(func);
    logUISink->set_level(spdlog::level::trace);
    logUISink->set_pattern("%v");
    Sinks->add_sink(logUISink);

    // Imgui

    ImGuiIO& io = ImGui::GetIO();
    //io.IniFilename = nullptr;   // don't save window positions etc. to ini
    //io.MouseDrawCursor = true; // let imgui draw cursors

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);

    // convert imgui style colors to linear RGB
    // since we have an sRGB backbuffer
    for(size_t i = 0; i < ImGuiCol_COUNT; i++)
    {
        glm::vec3 sRGB = glm::make_vec3(&style.Colors[i].x);
        glm::vec3 linear = glm::convertSRGBToLinear(sRGB);
        style.Colors[i] = ImVec4(linear.x, linear.y, linear.z, style.Colors[i].w);
    }

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
    ImFontConfig fontConfig;
    fontConfig.GlyphRanges = io.Fonts->GetGlyphRangesDefault(); // basic + extended Latin only
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontFile, 14.0f, &fontConfig);
    if(!font)
        io.Fonts->AddFontDefault();

    // Load and merge icon font
    const char* iconFontFile = "assets/fonts/ForkAwesome/forkawesome-webfont.ttf";
    static const ImWchar iconsRanges[] = { ICON_MIN_FK, ICON_MAX_16_FK, 0 }; // must persist for font lifetime
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.GlyphRanges = iconsRanges;
    iconsConfig.GlyphMinAdvanceX = 13.0f; // align icons
    iconsConfig.PixelSnapH = true;
    iconsConfig.OversampleH = 1;
    iconsConfig.OversampleV = 1;
    io.Fonts->AddFontFromFileTTF(iconFontFile, 13.0f, &iconsConfig);

    // Generate font texture
    unsigned char* tex_data = nullptr;
    int tex_w = 0, tex_h = 0;
    int bytes = 0;
    io.Fonts->GetTexDataAsRGBA32(&tex_data, &tex_w, &tex_h, &bytes);
    fontTexture = bgfx::createTexture2D((uint16_t)tex_w,
                                        (uint16_t)tex_h,
                                        false,
                                        1,
                                        bgfx::TextureFormat::RGBA8,
                                        0,
                                        bgfx::copy(tex_data, tex_w * tex_h * bytes));
    io.Fonts->SetTexID((ImTextureID)(uintptr_t)fontTexture.idx);
}

void ClusterUI::update(float dt)
{
    mTime += dt;
    if(!app.config->showUI)
        return;

    //ImGui::ShowDemoWindow();

    const Renderer::TextureBuffer* buffers = app.renderer->buffers;
    const ImVec2 padding = { 5.0f, 5.0f };

    if(app.config->showConfigWindow)
    {
        ImGui::Begin("Settings", &app.config->showConfigWindow, ImGuiWindowFlags_AlwaysAutoResize);

        if(ImGui::SliderInt("No. of lights", &app.config->lights, 0, app.config->maxLights))
            app.generateLights(app.config->lights);
        ImGui::Checkbox("Moving lights", &app.config->movingLights);

        ImGui::Separator();

        ImGui::SliderFloat("Exposure", &app.scene->camera.exposure, 0.0f, 30.0f, "%.3f");

        const char* operators[] = { "None",
                                    "Exponential",
                                    "Reinhard",
                                    "Reinhard - Luminance",
                                    "Hable (Uncharted 2)",
                                    "Duiker (Kodak curve)",
                                    "ACES",
                                    "ACES - Luminance" };
        int tonemappingMode = (int)app.config->tonemappingMode;
        ImGui::Combo("Tonemapping", &tonemappingMode, operators, IM_ARRAYSIZE(operators));
        app.config->tonemappingMode = (Renderer::TonemappingMode)tonemappingMode;
        app.renderer->setTonemappingMode(app.config->tonemappingMode);

        ImGui::Separator();

        ImGui::Checkbox("Multiple scattering", &app.config->multipleScattering);
        app.renderer->setMultipleScattering(app.config->multipleScattering);

        ImGui::Checkbox("White furnace", &app.config->whiteFurnace);
        ImGui::SameLine();
        ImGui::Text(ICON_FK_INFO_CIRCLE);
        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("Not implemented in the deferred renderer");
        app.renderer->setWhiteFurnace(app.config->whiteFurnace);

        ImGui::Separator();

        ImGui::Text("Render path:");
        static int renderPathSelected = (int)app.config->renderPath;
        ImGui::RadioButton("Forward", &renderPathSelected, (int)Cluster::RenderPath::Forward);
        ImGui::RadioButton("Deferred", &renderPathSelected, (int)Cluster::RenderPath::Deferred);
        ImGui::RadioButton("Clustered", &renderPathSelected, (int)Cluster::RenderPath::Clustered);
        Cluster::RenderPath path = (Cluster::RenderPath)renderPathSelected;
        if(path != app.config->renderPath)
            app.setRenderPath(path);

        ImGui::Separator();

        ImGui::Checkbox("Show log", &app.config->showLog);
        ImGui::Checkbox("Show performance stats", &app.config->showStatsOverlay);
        if(buffers)
            ImGui::Checkbox("Show G-Buffer", &app.config->showBuffers);
        if(path == Cluster::RenderPath::Clustered)
        {
            ImGui::Checkbox("Cluster light count visualization", &app.config->debugVisualization);
            app.renderer->setVariable("DEBUG_VIS", app.config->debugVisualization ? "true" : "false");
        }

        ImGui::Separator();

        if(ImGui::Button(app.config->fullscreen ? (ICON_FK_WINDOW_RESTORE "  Restore")
                                                : (ICON_FK_WINDOW_MAXIMIZE "  Fullscreen"),
                         ImVec2(100, 0)))
        {
            app.toggleFullscreen();
        }
        if(ImGui::Button(ICON_FK_EYE_SLASH "  Hide UI", ImVec2(100, 0)))
            app.config->showUI = false;
        ImGui::SameLine();
        std::string keyName = glfwGetKeyName(GLFW_KEY_R, 0);
        for(auto& c : keyName)
            c = std::toupper(c);
        ImGui::TextDisabled("%s to restore", keyName.c_str());
        ImGui::End();
    }

    // log
    if(app.config->showLog)
    {
        ImGui::Begin("Log", &app.config->showLog, ImGuiWindowFlags_HorizontalScrollbar);
        ImVec4 clrText = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImVec4 clrDisabled = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
        ImVec4 clrWarning = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // yellow/orange ("Gold")
        ImVec4 clrError = ImVec4(0.8f, 0.0f, 0.1f, 1.0f);   // yellow/orange ("Crimson")
        // trace, debug, info, warn, error, critical
        const ImVec4 colors[] = { clrDisabled, clrDisabled, clrText, clrWarning, clrError, clrError };
        const char* icons[] = { ICON_FK_INFO,        ICON_FK_INFO,        ICON_FK_INFO,
                                ICON_FK_EXCLAMATION, ICON_FK_EXCLAMATION, ICON_FK_EXCLAMATION };
        for(const LogEntry& entry : logEntries)
        {
            ImGui::TextColored(colors[entry.level], "%s %s", icons[entry.level], logText.begin() + entry.messageOffset);
        }
        // only scroll down if it's currently at the bottom
        // TODO this breaks scrolling up
        //if(ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        //    ImGui::SetScrollHereY(1.0f);
        ImGui::End();
    }

    // performance overlay
    if(app.config->showStatsOverlay)
    {
        const float overlayWidth = 150.0f;

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
        ImGui::TextDisabled("right-click to toggle");
        ImGui::Separator();

        // general data
        const bgfx::Stats* stats = bgfx::getStats();
        const double toCpuMs = 1000.0 / double(stats->cpuTimerFreq);
        const double toGpuMs = 1000.0 / double(stats->gpuTimerFreq);

        ImGui::Text("Backend: %s", bgfx::getRendererName(bgfx::getRendererType()));
        ImGui::Text("Buffer size: %u x %u px", stats->width, stats->height);
        ImGui::Text("Triangles: %u", stats->numPrims[bgfx::Topology::TriList]);
        ImGui::Text("Draw calls: %u", stats->numDraw);
        ImGui::Text("Compute calls: %u", stats->numCompute);

        // plots
        static float fpsValues[GRAPH_HISTORY] = { 0 };
        static float frameTimeValues[GRAPH_HISTORY] = { 0 };
        static float gpuMemoryValues[GRAPH_HISTORY] = { 0 };
        static size_t offset = 0;

        if(app.config->overlays.fps)
        {
            ImGui::Separator();
            ImGui::Text("FPS");
            ImGui::PlotLines("",
                             fpsValues,
                             IM_ARRAYSIZE(fpsValues),
                             (int)offset + 1,
                             nullptr,
                             0.0f,
                             200.0f,
                             ImVec2(overlayWidth, 50));
            ImGui::Text("%.0f", fpsValues[offset]);
        }
        if(app.config->overlays.frameTime)
        {
            ImGui::Separator();
            ImGui::Text("Frame time");
            ImGui::PlotLines("",
                             frameTimeValues,
                             IM_ARRAYSIZE(frameTimeValues),
                             (int)offset + 1,
                             nullptr,
                             0.0f,
                             30.0f,
                             ImVec2(overlayWidth, 50));
            ImGui::Text("CPU: %.2f ms", float(stats->cpuTimeEnd - stats->cpuTimeBegin) * toCpuMs);
            ImGui::Text("GPU: %.2f ms", float(stats->gpuTimeEnd - stats->gpuTimeBegin) * toGpuMs);
            ImGui::Text("Total: %.2f ms", frameTimeValues[offset]);
        }
        if(app.config->profile && app.config->overlays.profiler)
        {
            ImGui::Separator();
            ImGui::Text("View stats");
            if(stats->numViews > 0)
            {
                ImVec4 cpuColor(0.5f, 1.0f, 0.5f, 1.0f);
                ImVec4 gpuColor(0.5f, 0.5f, 1.0f, 1.0f);

                const float itemHeight = ImGui::GetTextLineHeightWithSpacing();
                const float itemHeightWithSpacing = ImGui::GetFrameHeightWithSpacing();
                const float scale = 2.0f;

                if(ImGui::BeginListBox("##ViewStats", ImVec2(overlayWidth, stats->numViews * itemHeightWithSpacing)))
                {
                    ImGuiListClipper clipper;
                    clipper.Begin(stats->numViews, itemHeight);

                    while(clipper.Step())
                    {
                        for(int32_t pos = clipper.DisplayStart; pos < clipper.DisplayEnd; ++pos)
                        {
                            const bgfx::ViewStats& viewStats = stats->viewStats[pos];
                            float cpuElapsed = float((viewStats.cpuTimeEnd - viewStats.cpuTimeBegin) * toCpuMs);
                            float gpuElapsed = float((viewStats.gpuTimeEnd - viewStats.gpuTimeBegin) * toGpuMs);

                            ImGui::Text("%d", viewStats.view);

                            const float maxWidth = overlayWidth * 0.35f;
                            const float cpuWidth = bx::clamp(cpuElapsed * scale, 1.0f, maxWidth);
                            const float gpuWidth = bx::clamp(gpuElapsed * scale, 1.0f, maxWidth);

                            ImGui::SameLine(overlayWidth * 0.3f);

                            if(drawBar("CPU", cpuWidth, maxWidth, itemHeight, cpuColor))
                            {
                                ImGui::SetTooltip("%s -- CPU: %.2f ms", viewStats.name, cpuElapsed);
                            }

                            ImGui::SameLine();

                            if(drawBar("GPU", gpuWidth, maxWidth, itemHeight, gpuColor))
                            {
                                ImGui::SetTooltip("%s -- GPU: %.2f ms", viewStats.name, gpuElapsed);
                            }
                        }
                    }

                    clipper.End();

                    ImGui::EndListBox();
                }
            }
            else
            {
                ImGui::TextWrapped(ICON_FK_EXCLAMATION_TRIANGLE " Profiler disabled");
            }
        }
        if(app.config->overlays.gpuMemory)
        {
            int64_t used = stats->gpuMemoryUsed;
            int64_t max = stats->gpuMemoryMax;

            ImGui::Separator();
            if(used > 0 && max > 0)
            {
                ImGui::Text("GPU memory");
                ImGui::PlotLines("",
                                 gpuMemoryValues,
                                 IM_ARRAYSIZE(gpuMemoryValues),
                                 (int)offset + 1,
                                 nullptr,
                                 0.0f,
                                 float(max),
                                 ImVec2(overlayWidth, 50));
                char strUsed[64];
                bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->gpuMemoryUsed);
                char strMax[64];
                bx::prettify(strMax, BX_COUNTOF(strMax), stats->gpuMemoryMax);
                ImGui::Text("%s / %s", strUsed, strMax);
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
            gpuMemoryValues[offset] = float(stats->gpuMemoryUsed) / 1024 / 1024;

            oldTime = mTime;
        }

        // right click menu
        if(ImGui::BeginPopupContextWindow())
        {
            ImGui::Checkbox("FPS", &app.config->overlays.fps);
            ImGui::Checkbox("Frame time", &app.config->overlays.frameTime);
            if(app.config->profile)
                ImGui::Checkbox("View stats", &app.config->overlays.profiler);
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

        ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 borderColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);

        for(size_t i = 0; buffers[i].name != nullptr; i++)
        {
            ImGui::Text("%s", buffers[i].name);
            ImTextureID texId = ImTextureID(uintptr_t(buffers[i].handle.idx));
            ImVec2 texSize = io.DisplaySize;
            texSize = { 128.0f, 128.0f };
            ImVec2 topLeft = ImVec2(0.0f, 0.0f);
            ImVec2 bottomRight = ImVec2(1.0f, 1.0f);
            if(bgfx::getCaps()->originBottomLeft)
            {
                std::swap(topLeft.y, bottomRight.y);
            }
            ImGui::Image(texId, texSize, topLeft, bottomRight, tintColor, borderColor);
        }

        // move window to bottom right
        ImVec2 displaySize = io.DisplaySize;
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 windowPos(displaySize.x - windowSize.x - padding.x, displaySize.y - windowSize.y - padding.y);
        ImGui::SetWindowPos(windowPos);

        ImGui::End();
    }
}

void ClusterUI::shutdown()
{
    logEntries.clear();
    logText.clear();
    ImGuiIO& io = ImGui::GetIO();
    // destroy font texture since we always create it ourselves
    bgfx::destroy(fontTexture);
    fontTexture = BGFX_INVALID_HANDLE;
    io.Fonts->SetTexID(ImTextureID(uintptr_t(fontTexture.idx)));
    Sinks->remove_sink(logUISink);
    logUISink = nullptr;
}

void ClusterUI::log(const char* message, spdlog::level::level_enum level)
{
    // TODO limit to 2000 entries
    // rolling index?
    int vecLen = logText.size();
    int32_t strLen = bx::strLen(message) + 1;
    logText.resize(vecLen + strLen);
    bx::memCopy(logText.begin() + vecLen, message, strLen);

    logEntries.push_back({ level, vecLen });
}

bool ClusterUI::drawBar(const char* id, float width, float maxWidth, float height, const ImVec4& color)
{
    const ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 hoveredColor(color.x * 1.1f, color.y * 1.1f, color.z * 1.1f, color.w * 1.1f);

    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, style.ItemSpacing.y));

    bool itemHovered = false;

    ImGui::PushID(id);

    ImGui::Button("##Visible", ImVec2(width, height));
    itemHovered = itemHovered || ImGui::IsItemHovered();

    ImGui::SameLine();
    ImGui::InvisibleButton("##Invisible", ImVec2(std::max(1.0f, maxWidth - width), height));
    itemHovered = itemHovered || ImGui::IsItemHovered();

    ImGui::PopID();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    return itemHovered;
}
