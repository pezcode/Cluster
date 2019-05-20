#pragma once

#include <bgfx/bgfx.h>
#include "Renderer/PBRShader.h"

class Scene;

class LightShader
{
public:
    static constexpr uint8_t SAMPLER_START = PBRShader::SAMPLER_END + 1;
    static constexpr uint8_t SAMPLER_END = SAMPLER_START + 1;

    LightShader();

    void initialize();
    void shutdown();

    uint64_t bindLights(const Scene* scene) const;

private:
    bgfx::UniformHandle lightCountVecUniform;
};
