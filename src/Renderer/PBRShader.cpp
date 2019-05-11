#include "PBRShader.h"

#include "Scene/Material.h"

PBRShader::PBRShader() :
    baseColorUniform(BGFX_INVALID_HANDLE),
    metallicRoughnessUniform(BGFX_INVALID_HANDLE),
    hasTexturesUniform(BGFX_INVALID_HANDLE),
    baseColorSampler(BGFX_INVALID_HANDLE),
    metallicRoughnessSampler(BGFX_INVALID_HANDLE),
    normalSampler(BGFX_INVALID_HANDLE)
{
}

void PBRShader::init()
{
    baseColorUniform = bgfx::createUniform("u_baseColor", bgfx::UniformType::Vec4);
    metallicRoughnessUniform = bgfx::createUniform("u_metallicRoughness", bgfx::UniformType::Vec4);
    hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
    baseColorSampler = bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler);
    metallicRoughnessSampler = bgfx::createUniform("s_texMetallicRoughness", bgfx::UniformType::Sampler);
    normalSampler = bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler);
}

void PBRShader::shutdown()
{
    bgfx::destroy(baseColorUniform);
    bgfx::destroy(metallicRoughnessUniform);
    bgfx::destroy(hasTexturesUniform);
    bgfx::destroy(baseColorSampler);
    bgfx::destroy(metallicRoughnessSampler);
    bgfx::destroy(normalSampler);

    baseColorUniform = metallicRoughnessUniform = hasTexturesUniform = baseColorSampler = metallicRoughnessSampler = normalSampler = BGFX_INVALID_HANDLE;
}

uint64_t PBRShader::bindMaterial(const Material& material)
{
    float metallicRoughnessValues[4] = { material.metallic, material.roughness, 0.0f, 0.0f };
    float hasTexturesValues[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
    bgfx::setUniform(baseColorUniform, &material.baseColor[0]);
    bgfx::setUniform(metallicRoughnessUniform, &metallicRoughnessValues[0]);

    if(bgfx::isValid(material.baseColorTexture))
        bgfx::setTexture(0, baseColorSampler, material.baseColorTexture);
    else
        hasTexturesValues[0] = 0.0f;
    if(bgfx::isValid(material.metallicRoughnessTexture))
        bgfx::setTexture(1, metallicRoughnessSampler, material.metallicRoughnessTexture);
    else
        hasTexturesValues[1] = 0.0f;
    if(bgfx::isValid(material.normalTexture))
        bgfx::setTexture(2, normalSampler, material.normalTexture);
    else
        hasTexturesValues[2] = 0.0f;

    bgfx::setUniform(hasTexturesUniform, &hasTexturesValues[0]);

    uint64_t state = 0;
    if(material.blend)
        state |= BGFX_STATE_BLEND_ALPHA;
    if(!material.doubleSided)
        state |= BGFX_STATE_CULL_CW;
    return state;
}
