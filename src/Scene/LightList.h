#pragma once

#include "Scene/Light.h"
#include <bgfx/bgfx.h>
#include <vector>
#include <memory>

struct LightList
{
    struct Vec3Vertex
    {
        float x;
        float y;
        float z;
        // we only need 3 floats but the buffer values end up in funny ways when using 3
        // not sure if this is a problem with padding/stride on the CPU or if buffers need to be padded to vec4
        float padding;

        static void init()
        {
            decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .skip(sizeof(float))
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

    bgfx::DynamicVertexBufferHandle positionBuffer;
    bgfx::DynamicVertexBufferHandle fluxBuffer;

private:
    std::unique_ptr<Vec3Vertex[]> position;
    std::unique_ptr<Vec3Vertex[]> flux;
};
