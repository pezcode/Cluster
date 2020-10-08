#pragma once

#include <bgfx/bgfx.h>

struct Material;

class PBRShader
{
public:
    void initialize();
    void shutdown();

    void setMultipleScattering(bool enabled);

    void generateAlbedoLUT();

    uint64_t bindMaterial(const Material& material);
    void bindAlbedoLUT(bool compute = false);

private:
    static constexpr uint16_t ALBEDO_LUT_SIZE = 32;
    static constexpr uint16_t ALBEDO_LUT_THREADS = 32;

    bool multipleScatteringEnabled = true;

    bgfx::UniformHandle baseColorFactorUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle metallicRoughnessNormalOcclusionFactorUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle emissiveFactorUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle hasTexturesUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle albedoLUTSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle baseColorSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle metallicRoughnessSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle normalSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle occlusionSampler = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle emissiveSampler = BGFX_INVALID_HANDLE;

    bgfx::TextureHandle albedoLUTTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle albedoLUTNoMSTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle defaultTexture = BGFX_INVALID_HANDLE;

    bgfx::ProgramHandle albedoLUTProgram = BGFX_INVALID_HANDLE;
};
