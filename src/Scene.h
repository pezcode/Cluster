#pragma once

#include "Camera.h"
#include <bgfx/bgfx.h>

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

class Scene
{
public:
    Scene();

    void load(const char* path);

    PosColorVertex s_cubeVertices[8];
    const uint16_t s_cubeTriList[36];
    Camera camera;
};
