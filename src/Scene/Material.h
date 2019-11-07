#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <bgfx/bgfx.h>

// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#metallic-roughness-material
struct Material
{
    bool blend = false;
    bool doubleSided = false;

    bgfx::TextureHandle baseColorTexture = BGFX_INVALID_HANDLE;
    glm::vec4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };

    bgfx::TextureHandle metallicRoughnessTexture = BGFX_INVALID_HANDLE; // blue = metallic, green = roughness
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;

    bgfx::TextureHandle normalTexture = BGFX_INVALID_HANDLE;
    float normalScale = 1.0f;

    bgfx::TextureHandle occlusionTexture = BGFX_INVALID_HANDLE;
    float occlusionStrength = 1.0f;

    bgfx::TextureHandle emissiveTexture = BGFX_INVALID_HANDLE;
    glm::vec3 emissiveFactor = { 0.0f, 0.0f, 0.0f };
};
