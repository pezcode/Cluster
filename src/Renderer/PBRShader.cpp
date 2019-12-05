#include "PBRShader.h"

#include "Scene/Material.h"
#include "Renderer/Samplers.h"
#include <glm/gtc/type_ptr.hpp>

PBRShader::PBRShader() :
    baseColorFactorUniform(BGFX_INVALID_HANDLE),
    metallicRoughnessNormalOcclusionFactorUniform(BGFX_INVALID_HANDLE),
    emissiveFactorUniform(BGFX_INVALID_HANDLE),
    hasTextures1Uniform(BGFX_INVALID_HANDLE),
    hasTextures2Uniform(BGFX_INVALID_HANDLE),
    baseColorSampler(BGFX_INVALID_HANDLE),
    metallicRoughnessSampler(BGFX_INVALID_HANDLE),
    normalSampler(BGFX_INVALID_HANDLE),
    occlusionSampler(BGFX_INVALID_HANDLE),
    emissiveSampler(BGFX_INVALID_HANDLE)
{
}

void PBRShader::initialize()
{
    baseColorFactorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);
    metallicRoughnessNormalOcclusionFactorUniform =
        bgfx::createUniform("u_metallicRoughnessNormalOcclusionFactor", bgfx::UniformType::Vec4);
    emissiveFactorUniform = bgfx::createUniform("u_emissiveFactorVec", bgfx::UniformType::Vec4);
    hasTextures1Uniform = bgfx::createUniform("u_hasTextures1", bgfx::UniformType::Vec4);
    hasTextures2Uniform = bgfx::createUniform("u_hasTextures2", bgfx::UniformType::Vec4);
    baseColorSampler = bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler);
    metallicRoughnessSampler = bgfx::createUniform("s_texMetallicRoughness", bgfx::UniformType::Sampler);
    normalSampler = bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler);
    occlusionSampler = bgfx::createUniform("s_texOcclusion", bgfx::UniformType::Sampler);
    emissiveSampler = bgfx::createUniform("s_texEmissive", bgfx::UniformType::Sampler);
}

void PBRShader::shutdown()
{
    bgfx::destroy(baseColorFactorUniform);
    bgfx::destroy(metallicRoughnessNormalOcclusionFactorUniform);
    bgfx::destroy(emissiveFactorUniform);
    bgfx::destroy(hasTextures1Uniform);
    bgfx::destroy(hasTextures2Uniform);
    bgfx::destroy(baseColorSampler);
    bgfx::destroy(metallicRoughnessSampler);
    bgfx::destroy(normalSampler);
    bgfx::destroy(occlusionSampler);
    bgfx::destroy(emissiveSampler);

    baseColorFactorUniform = metallicRoughnessNormalOcclusionFactorUniform = emissiveFactorUniform =
        hasTextures1Uniform = hasTextures2Uniform = baseColorSampler = metallicRoughnessSampler = normalSampler =
            occlusionSampler = emissiveSampler = BGFX_INVALID_HANDLE;
}

uint64_t PBRShader::bindMaterial(const Material& material)
{
    float factorValues[4] = {
        material.metallicFactor, material.roughnessFactor, material.normalScale, material.occlusionStrength
    };
    bgfx::setUniform(baseColorFactorUniform, glm::value_ptr(material.baseColorFactor));
    bgfx::setUniform(metallicRoughnessNormalOcclusionFactorUniform, factorValues);
    glm::vec4 emissiveFactor = glm::vec4(material.emissiveFactor, 0.0f);
    bgfx::setUniform(emissiveFactorUniform, glm::value_ptr(emissiveFactor));

    float hasTextures1Values[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float hasTextures2Values[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    if(bgfx::isValid(material.baseColorTexture))
        bgfx::setTexture(Samplers::PBR_BASECOLOR, baseColorSampler, material.baseColorTexture);
    else
        hasTextures1Values[0] = 0.0f;
    if(bgfx::isValid(material.metallicRoughnessTexture))
        bgfx::setTexture(Samplers::PBR_METALROUGHNESS, metallicRoughnessSampler, material.metallicRoughnessTexture);
    else
        hasTextures1Values[1] = 0.0f;
    if(bgfx::isValid(material.normalTexture))
        bgfx::setTexture(Samplers::PBR_NORMAL, normalSampler, material.normalTexture);
    else
        hasTextures1Values[2] = 0.0f;
    if(bgfx::isValid(material.occlusionTexture))
        bgfx::setTexture(Samplers::PBR_OCCLUSION, occlusionSampler, material.occlusionTexture);
    else
        hasTextures1Values[3] = 0.0f;
    if(bgfx::isValid(material.emissiveTexture))
        bgfx::setTexture(Samplers::PBR_EMISSIVE, emissiveSampler, material.emissiveTexture);
    else
        hasTextures2Values[0] = 0.0f;

    bgfx::setUniform(hasTextures1Uniform, hasTextures1Values);
    bgfx::setUniform(hasTextures2Uniform, hasTextures2Values);

    uint64_t state = 0;
    if(material.blend)
        state |= BGFX_STATE_BLEND_ALPHA;
    if(!material.doubleSided)
        state |= BGFX_STATE_CULL_CW;
    return state;
}
