#include "LightShader.h"

#include "Scene/Scene.h"
#include "Renderer/Samplers.h"
#include <cassert>

LightShader::LightShader() :
    lightCountVecUniform(BGFX_INVALID_HANDLE),
    ambientLightIrradianceUniform(BGFX_INVALID_HANDLE)
{
}

void LightShader::initialize()
{
    lightCountVecUniform = bgfx::createUniform("u_lightCountVec", bgfx::UniformType::Vec4);
    ambientLightIrradianceUniform = bgfx::createUniform("u_ambientLightIrradiance", bgfx::UniformType::Vec4);
}

void LightShader::shutdown()
{
    bgfx::destroy(lightCountVecUniform);
    bgfx::destroy(ambientLightIrradianceUniform);

    lightCountVecUniform = ambientLightIrradianceUniform = BGFX_INVALID_HANDLE;
}

void LightShader::bindLights(const Scene* scene) const
{
    assert(scene != nullptr);

    // a 32-bit IEEE 754 float can represent all integers up to 2^24 (~16.7 million) correctly
    // should be enough for this use case (comparison in for loop)
    float lightCountVec[4] = { (float)scene->pointLights.lights.size() };
    bgfx::setUniform(lightCountVecUniform, lightCountVec);

    float ambientLightIrradiance[4] = { scene->ambientLight.irradiance.r,
                                        scene->ambientLight.irradiance.g,
                                        scene->ambientLight.irradiance.b };
    bgfx::setUniform(ambientLightIrradianceUniform, ambientLightIrradiance);

    bgfx::setBuffer(Samplers::LIGHTS_POINTLIGHT_POSITION, scene->pointLights.positionBuffer, bgfx::Access::Read);
    bgfx::setBuffer(Samplers::LIGHTS_POINTLIGHT_POWER,    scene->pointLights.powerBuffer,    bgfx::Access::Read);
}
