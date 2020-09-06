#pragma once

#include <bgfx/bgfx.h>

class Scene;

class LightShader
{
public:
    void initialize();
    void shutdown();

    void bindLights(const Scene* scene) const;

private:
    bgfx::UniformHandle lightCountVecUniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle ambientLightIrradianceUniform = BGFX_INVALID_HANDLE;
};
