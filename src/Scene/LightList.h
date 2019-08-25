#pragma once

#include "Scene/Light.h"
#include <bgfx/bgfx.h>
#include <vector>

struct LightList
{
    // vertex buffers seem to be aligned to 16 bytes
    struct PointLightVertex
    {
        glm::vec3 position;
        float padding;
        glm::vec3 power;
        float radius;

        static void init()
        {
            decl.begin()
                .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
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

    bgfx::DynamicVertexBufferHandle buffer;
};
