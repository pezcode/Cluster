#include "LightShader.h"

#include "Scene/Scene.h"
#include "Renderer/Samplers.h"
#include <glm/gtc/type_ptr.hpp>
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

    glm::vec4 ambientLightIrradiance(scene->ambientLight.irradiance, 1.0f);
    bgfx::setUniform(ambientLightIrradianceUniform, glm::value_ptr(ambientLightIrradiance));

    bgfx::setBuffer(Samplers::LIGHTS_POINTLIGHTS, scene->pointLights.buffer, bgfx::Access::Read);
}
