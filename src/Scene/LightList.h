#pragma once

#include "Scene/Light.h"
#include <bgfx/bgfx.h>
#include <vector>
#include <memory>

struct LightList
{
    // vertex buffers seem to be aligned to 16 bytes
    struct Vec4Vertex
    {
        float x;
        float y;
        float z;
        float w;

        static void init()
        {
            decl.begin()
                .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
                .end();
        }
        static bgfx::VertexDecl decl;
    };
};

class PointLightList : public LightList
{
public:
    PointLightList();

    std::vector<PointLight> lights;

    void init();
    void shutdown();

    // upload changes to GPU
    void update();

    bgfx::VertexBufferHandle positionBuffer;
    bgfx::VertexBufferHandle powerBuffer;
};
