#pragma once

#include <bgfx/bgfx.h>
#include "Renderer/PBRShader.h"

class LightShader
{
public:
    static constexpr uint8_t SAMPLER_START = PBRShader::SAMPLER_END + 1;
    static constexpr uint8_t SAMPLER_END = SAMPLER_START + 1;

    LightShader();

    void initialize();
    void shutdown();

    uint64_t bindLights();

private:
    bgfx::UniformHandle lightCountVecUniform;
    bgfx::DynamicVertexBufferHandle lightPosBuffer;
    bgfx::DynamicVertexBufferHandle lightFluxBuffer;

    struct VertexVec3
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

    uint32_t numLights;

    VertexVec3* lightPositions;
    VertexVec3* lightFluxs;
};
