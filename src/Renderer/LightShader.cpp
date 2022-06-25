#include "LightShader.h"

#include "Scene/Scene.h"
#include "Renderer/Samplers.h"
#include <glm/gtc/type_ptr.hpp>
#include <cassert>

void LightShader::initialize()
{
    lightCountVecUniform = bgfx::createUniform("u_lightCountVec", bgfx::UniformType::Vec4);
    sunLightDirectionUniform = bgfx::createUniform("u_sunLightDirection", bgfx::UniformType::Vec4);
    sunLightRadianceUniform = bgfx::createUniform("u_sunLightRadiance", bgfx::UniformType::Vec4);
    ambientLightIrradianceUniform = bgfx::createUniform("u_ambientLightIrradiance", bgfx::UniformType::Vec4);
}

void LightShader::shutdown()
{
    bgfx::destroy(lightCountVecUniform);
    bgfx::destroy(sunLightDirectionUniform);
    bgfx::destroy(sunLightRadianceUniform);
    bgfx::destroy(ambientLightIrradianceUniform);

    lightCountVecUniform = sunLightDirectionUniform = sunLightRadianceUniform = ambientLightIrradianceUniform = BGFX_INVALID_HANDLE;
}

void LightShader::bindLights(const Scene* scene) const
{
    assert(scene != nullptr);

    // a 32-bit IEEE 754 float can represent all integers up to 2^24 (~16.7 million) correctly
    // should be enough for this use case (comparison in for loop)
    float lightCountVec[4] = { (float)scene->pointLights.lights.size() };
    bgfx::setUniform(lightCountVecUniform, lightCountVec);

    glm::vec4 sunLightDirection(glm::normalize(scene->sunLight.direction), 0.0f);
    bgfx::setUniform(sunLightDirectionUniform, glm::value_ptr(sunLightDirection));
    glm::vec4 sunLightRadiance(scene->sunLight.radiance, 1.0f);
    bgfx::setUniform(sunLightRadianceUniform, glm::value_ptr(sunLightRadiance));

    glm::vec4 ambientLightIrradiance(scene->ambientLight.irradiance, 1.0f);
    bgfx::setUniform(ambientLightIrradianceUniform, glm::value_ptr(ambientLightIrradiance));

    bgfx::setBuffer(Samplers::LIGHTS_POINTLIGHTS, scene->pointLights.buffer, bgfx::Access::Read);
}
