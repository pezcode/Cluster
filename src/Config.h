#pragma once

class Config
{
public:
    bool showUI;
    bool showConfigWindow;
    bool showStatsOverlay;
    struct
    {
        bool fps;
        bool frameTime;
        bool gpuMemory;
    } overlays;
    bool showBuffers;

    void readArgv(int argc, char* argv[]);
};
