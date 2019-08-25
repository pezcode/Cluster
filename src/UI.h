#pragma once

#include <bgfx/bgfx.h>
#include <imgui.h>
#include <spdlog/common.h>

class Cluster;

class ClusterUI
{
public:
    ClusterUI(Cluster& app);
    ~ClusterUI();

    void initialize();
    void update(float dt);
    void shutdown();

    void log(const char* message, spdlog::level::level_enum level = spdlog::level::info);

private:
    void imageTooltip(ImTextureID tex, ImVec2 tex_size, float region_size, ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4 borderColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

    struct LogEntry
    {
        spdlog::level::level_enum level;
        int messageOffset; // points into vector of char
    };

    ImVector<LogEntry> logEntries;
    ImVector<char> logText;

    // update 10 times per second
    static constexpr float GRAPH_FREQUENCY = 0.1f;
    // show 100 values
    static constexpr size_t GRAPH_HISTORY = 100;

    Cluster& app;
    float mTime;

    bgfx::TextureHandle fontTexture;
};
