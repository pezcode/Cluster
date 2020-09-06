#pragma once

#include <bgfx/bgfx.h>

struct Material;

class PBRShader
{
public:
    void initialize();
    void shutdown();

    uint64_t bindMaterial(const Material& material);

private:
    bgfx::UniformHandle baseColorFactorUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle metallicRoughnessNormalOcclusionFactorUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle emissiveFactorUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle hasTexturesUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle baseColorSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle metallicRoughnessSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle normalSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle occlusionSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle emissiveSampler = BGFX_INVALID_HANDLE;

    bgfx::TextureHandle defaultTexture = BGFX_INVALID_HANDLE;
};
