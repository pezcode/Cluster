#pragma once

#include <bgfx/bgfx.h>

struct Material;

class PBRShader
{
public:

    PBRShader();

    void init();
    void shutdown();

    uint64_t bindMaterial(const Material& material);

private:
    bgfx::UniformHandle baseColorUniform;
    bgfx::UniformHandle metallicRoughnessUniform;
    bgfx::UniformHandle hasTexturesUniform;
    bgfx::UniformHandle baseColorSampler;
    bgfx::UniformHandle metallicRoughnessSampler;
    bgfx::UniformHandle normalSampler;
};
