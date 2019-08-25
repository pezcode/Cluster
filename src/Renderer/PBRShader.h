#pragma once

#include <bgfx/bgfx.h>

struct Material;

class PBRShader
{
public:
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
