#pragma once

#include <glm/vec4.hpp>
#include <bgfx/bgfx.h>

// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#metallic-roughness-material
struct Material
{
    bool blend = false;
    bool doubleSided = false;
    glm::vec4 baseColor = { 1.0f, 0.0f, 1.0f, 1.0f }; // normalized RGBA
    float metallic = 0.0f;
    float roughness = 0.5f;
    bgfx::TextureHandle baseColorTexture = BGFX_INVALID_HANDLE;         // F0 for non-metals
    bgfx::TextureHandle metallicRoughnessTexture = BGFX_INVALID_HANDLE; // blue = metallic, green = roughness
    bgfx::TextureHandle normalTexture = BGFX_INVALID_HANDLE;
};
