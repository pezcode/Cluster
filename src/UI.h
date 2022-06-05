#pragma once

#include <bgfx/bgfx.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

class Cluster;

class ClusterUI
{
public:
    ClusterUI(Cluster& app);

    void initialize();
    void update(float dt);
    void shutdown();

    void log(const char* message, spdlog::level::level_enum level = spdlog::level::info);

private:
    bool drawBar(const char* id, float width, float maxWidth, float height, const ImVec4& color);

    spdlog::sink_ptr logUISink = nullptr;

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
    float mTime = 0.0f;

    bgfx::TextureHandle fontTexture = BGFX_INVALID_HANDLE;
};
