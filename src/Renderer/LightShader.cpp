#include "LightShader.h"

#include "Scene/Scene.h"
#include <cassert>

LightShader::LightShader() :
    lightCountVecUniform(BGFX_INVALID_HANDLE)
{
}

void LightShader::initialize()
{
    lightCountVecUniform = bgfx::createUniform("u_lightCountVec", bgfx::UniformType::Vec4);   
}

void LightShader::shutdown()
{
    bgfx::destroy(lightCountVecUniform);
    lightCountVecUniform = BGFX_INVALID_HANDLE;
}

uint64_t LightShader::bindLights(const Scene* scene) const
{
    assert(scene != nullptr);

    // a 32-bit IEEE 754 float can represent all integers up to 2^24 (~16.7 million) correctly
    // should be enough for this use case (comparison in for loop)
    float lightCountVec[4] = { (float)scene->pointLights.lights.size() };
    bgfx::setUniform(lightCountVecUniform, lightCountVec);

    bgfx::setBuffer(SAMPLER_START,     scene->pointLights.positionBuffer, bgfx::Access::Read);
    bgfx::setBuffer(SAMPLER_START + 1, scene->pointLights.powerBuffer,     bgfx::Access::Read);

    return 0;
}
