#include "PBRShader.h"

#include "Scene/Material.h"
#include "Renderer/Renderer.h"
#include "Renderer/Samplers.h"
#include <bigg.hpp>
#include <bx/string.h>
#include <bimg/encode.h>
#include <bx/file.h>
#include <glm/gtc/type_ptr.hpp>

void PBRShader::initialize()
{
    baseColorFactorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);
    metallicRoughnessNormalOcclusionFactorUniform =
        bgfx::createUniform("u_metallicRoughnessNormalOcclusionFactor", bgfx::UniformType::Vec4);
    emissiveFactorUniform = bgfx::createUniform("u_emissiveFactorVec", bgfx::UniformType::Vec4);
    hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
    albedoLUTSampler = bgfx::createUniform("s_texAlbedoLUT", bgfx::UniformType::Sampler);
    baseColorSampler = bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler);
    metallicRoughnessSampler = bgfx::createUniform("s_texMetallicRoughness", bgfx::UniformType::Sampler);
    normalSampler = bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler);
    occlusionSampler = bgfx::createUniform("s_texOcclusion", bgfx::UniformType::Sampler);
    emissiveSampler = bgfx::createUniform("s_texEmissive", bgfx::UniformType::Sampler);

    const uint64_t samplerFlags = BGFX_SAMPLER_UVW_CLAMP;

    albedoLUTTexture = bgfx::createTexture2D(ALBEDO_LUT_SIZE,
                                             ALBEDO_LUT_SIZE,
                                             false,
                                             1,
                                             bgfx::TextureFormat::RGBA32F,
                                             samplerFlags | BGFX_TEXTURE_COMPUTE_WRITE);

    // LUT with all 1s = no multiple scattering
    const bgfx::Memory* mem = bgfx::alloc(ALBEDO_LUT_SIZE * ALBEDO_LUT_SIZE * 4 * sizeof(float));
    //memset(mem->data, 0xFF, mem->size);
    float* data = (float*)mem->data;
    std::fill(data, data + (ALBEDO_LUT_SIZE * ALBEDO_LUT_SIZE * 4), 1.0f);
    albedoLUTNoMSTexture = bgfx::createTexture2D(
        ALBEDO_LUT_SIZE, ALBEDO_LUT_SIZE, false, 1, bgfx::TextureFormat::RGBA32F, samplerFlags, mem);

    defaultTexture = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::RGBA8);

    char csName[128];
    bx::snprintf(csName, BX_COUNTOF(csName), "%s%s", Renderer::shaderDir(), "cs_multiple_scattering_lut.bin");
    albedoLUTProgram = bgfx::createProgram(bigg::loadShader(csName), true);
}

void PBRShader::shutdown()
{
    bgfx::destroy(baseColorFactorUniform);
    bgfx::destroy(metallicRoughnessNormalOcclusionFactorUniform);
    bgfx::destroy(emissiveFactorUniform);
    bgfx::destroy(hasTexturesUniform);
    bgfx::destroy(albedoLUTSampler);
    bgfx::destroy(baseColorSampler);
    bgfx::destroy(metallicRoughnessSampler);
    bgfx::destroy(normalSampler);
    bgfx::destroy(occlusionSampler);
    bgfx::destroy(emissiveSampler);
    bgfx::destroy(albedoLUTTexture);
    bgfx::destroy(albedoLUTNoMSTexture);
    bgfx::destroy(defaultTexture);
    bgfx::destroy(albedoLUTProgram);

    baseColorFactorUniform = metallicRoughnessNormalOcclusionFactorUniform = emissiveFactorUniform =
        hasTexturesUniform = albedoLUTSampler = baseColorSampler = metallicRoughnessSampler = normalSampler =
            occlusionSampler = emissiveSampler = BGFX_INVALID_HANDLE;
    albedoLUTTexture = albedoLUTNoMSTexture = defaultTexture = BGFX_INVALID_HANDLE;
    albedoLUTProgram = BGFX_INVALID_HANDLE;
}

void PBRShader::setMultipleScattering(bool enabled)
{
    multipleScatteringEnabled = enabled;
}

void PBRShader::generateAlbedoLUT()
{
    const size_t w = ALBEDO_LUT_SIZE, h = ALBEDO_LUT_SIZE;

    bx::Error err;
    static const char* file = "albedo.tga";

    bool generate = true;
    bx::FileReader reader;
    if(reader.open(file, &err))
    {
        // TODO load cached albedo
        // can we blit to a compute texture?
        //generate = false;

        reader.close();
    }

    if(generate)
    {
        bindAlbedoLUT(true /* compute */);
        bgfx::dispatch(
            0, albedoLUTProgram, w / ALBEDO_LUT_THREADS, h / ALBEDO_LUT_THREADS, 1);

        // this code is for RGBA8
        /*

        // read-back is not supported on Vulkan
        const bgfx::Caps* caps = bgfx::getCaps();       
        if((caps->supported & BGFX_CAPS_TEXTURE_BLIT)      != 0 &&
           (caps->supported & BGFX_CAPS_TEXTURE_READ_BACK) != 0)
        {
            bgfx::TextureHandle readbackTexture = bgfx::createTexture2D(
                w, h, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK);

            bgfx::blit(1, readbackTexture, 0, 0, albedoLUTTexture);

            char* mem = new char[w * h * 4];
            bgfx::readTexture(readbackTexture, mem, 0);

            // wait two frames for result to be available and write it to a file
            bgfx::frame();
            bgfx::frame();

            bx::FileWriter writer;
            if(writer.open(file, false, &err))
            {
                bimg::imageWriteTga(&writer,
                                    w,
                                    h,
                                    w * 4, // stride
                                    mem,
                                    false, // not grayscale
                                    false, // don't flip y
                                    &err);
                writer.close();
            }

            delete[] mem;
            bgfx::destroy(readbackTexture);
        }
        */
    }
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

    float hasTexturesValues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    auto setTextureOrDefault = [&](uint8_t stage, bgfx::UniformHandle uniform, bgfx::TextureHandle texture) -> bool {
        bool valid = bgfx::isValid(texture);
        if(!valid)
            texture = defaultTexture;
        bgfx::setTexture(stage, uniform, texture);
        return valid;
    };

    const uint32_t hasTexturesMask = 0
        | ((setTextureOrDefault(Samplers::PBR_BASECOLOR, baseColorSampler, material.baseColorTexture) ? 1 : 0) << 0)
        | ((setTextureOrDefault(Samplers::PBR_METALROUGHNESS, metallicRoughnessSampler, material.metallicRoughnessTexture) ? 1 : 0) << 1)
        | ((setTextureOrDefault(Samplers::PBR_NORMAL, normalSampler, material.normalTexture) ? 1 : 0) << 2)
        | ((setTextureOrDefault(Samplers::PBR_OCCLUSION, occlusionSampler, material.occlusionTexture) ? 1 : 0) << 3)
        | ((setTextureOrDefault(Samplers::PBR_EMISSIVE, emissiveSampler, material.emissiveTexture) ? 1 : 0) << 4);
    hasTexturesValues[0] = static_cast<float>(hasTexturesMask);

    bgfx::setUniform(hasTexturesUniform, hasTexturesValues);

    uint64_t state = 0;
    if(material.blend)
        state |= BGFX_STATE_BLEND_ALPHA;
    if(!material.doubleSided)
        state |= BGFX_STATE_CULL_CW;
    return state;
}

void PBRShader::bindAlbedoLUT(bool compute)
{
    bgfx::TextureHandle tex = (multipleScatteringEnabled || compute) ? albedoLUTTexture : albedoLUTNoMSTexture;
    if(compute)
        bgfx::setImage(Samplers::PBR_ALBEDO_LUT, tex, 0, bgfx::Access::Write);
    else
        bgfx::setTexture(Samplers::PBR_ALBEDO_LUT, albedoLUTSampler, tex);
}
