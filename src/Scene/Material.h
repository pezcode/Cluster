#pragma once

#include <glm/vec4.hpp>
#include <bgfx/bgfx.h>

// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#metallic-roughness-material
struct Material
{
    bool blend = false;
    bool doubleSided = false;
    glm::vec4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    bgfx::TextureHandle baseColorTexture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle metallicRoughnessTexture = BGFX_INVALID_HANDLE; // blue = metallic, green = roughness
    bgfx::TextureHandle normalTexture = BGFX_INVALID_HANDLE;
};
