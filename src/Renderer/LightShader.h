#pragma once

#include <bgfx/bgfx.h>

class Scene;

class LightShader
{
public:
    LightShader();

    void initialize();
    void shutdown();

    void bindLights(const Scene* scene) const;

private:
    bgfx::UniformHandle lightCountVecUniform;
    bgfx::UniformHandle ambientLightIrradianceUniform;
};
