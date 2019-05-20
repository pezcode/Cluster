#pragma once

#include <bgfx/bgfx.h>

struct Material;

class PBRShader
{
public:

    static constexpr uint8_t SAMPLER_START = 0;
    static constexpr uint8_t SAMPLER_END   = SAMPLER_START + 2;

    PBRShader();

    void initialize();
    void shutdown();

    uint64_t bindMaterial(const Material& material);

private:
    bgfx::UniformHandle baseColorFactorUniform;
    bgfx::UniformHandle metallicRoughnessFactorUniform;
    bgfx::UniformHandle hasTexturesUniform;
    bgfx::UniformHandle baseColorSampler;
    bgfx::UniformHandle metallicRoughnessSampler;
    bgfx::UniformHandle normalSampler;
};
