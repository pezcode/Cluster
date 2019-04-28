#pragma once

#include <bx/string.h>
#include <bigg.hpp>

class Cluster : public bigg::Application
{
public:

    Cluster() : bigg::Application("Cluster", 1280, 720) { }

    void initialize(int _argc, char* _argv[]);

    void onReset();

    void update(float dt);

private:

    bgfx::ProgramHandle mProgram;
    bgfx::VertexBufferHandle mVbh;
    bgfx::IndexBufferHandle mIbh;

    float mTime;
};
