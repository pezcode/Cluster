#include "LightShader.h"

bgfx::VertexDecl LightShader::VertexVec3::decl;

LightShader::LightShader() :
    numLights(3),
    lightCountVecUniform(BGFX_INVALID_HANDLE),
    lightPosBuffer(BGFX_INVALID_HANDLE),
    lightFluxBuffer(BGFX_INVALID_HANDLE),
    lightPositions(nullptr),
    lightFluxs(nullptr)
{
}

void LightShader::initialize()
{
    VertexVec3::init();

    lightCountVecUniform = bgfx::createUniform("u_lightCountVec", bgfx::UniformType::Vec4);   

    lightPositions = new VertexVec3[numLights];
    lightFluxs = new VertexVec3[numLights];

    lightPosBuffer = bgfx::createDynamicVertexBuffer(numLights, VertexVec3::decl, BGFX_BUFFER_COMPUTE_READ);
    lightFluxBuffer = bgfx::createDynamicVertexBuffer(numLights, VertexVec3::decl, BGFX_BUFFER_COMPUTE_READ);

    lightPositions[0] = { -5.0f, 1.1f, 0.0f };
    lightPositions[1] = { 0.0f, 1.1f, 0.0f };
    lightPositions[2] = { 5.0f, 1.1f, 0.0f };

    lightFluxs[1] = { 1.0f, 0.0f, 1.0f };
    lightFluxs[0] = { 0.0f, 1.0f, 1.0f };
    lightFluxs[2] = { 1.0f, 1.0f, 0.0f };

    //uint32_t structsize = sizeof(VertexVec3);
    //uint32_t declsize = VertexVec3::decl.getStride();

    bgfx::update(lightPosBuffer,  0, bgfx::makeRef(lightPositions, sizeof(VertexVec3) * numLights));
    bgfx::update(lightFluxBuffer, 0, bgfx::makeRef(lightFluxs,     sizeof(VertexVec3) * numLights));
}

void LightShader::shutdown()
{
    bgfx::destroy(lightCountVecUniform);
    bgfx::destroy(lightPosBuffer);
    bgfx::destroy(lightFluxBuffer);

    lightPosBuffer = lightFluxBuffer = BGFX_INVALID_HANDLE;
    lightCountVecUniform = BGFX_INVALID_HANDLE;

    delete[] lightPositions;
    delete[] lightFluxs;
    lightPositions = lightFluxs = nullptr;
}

uint64_t LightShader::bindLights()
{
    // a 32-bit IEEE 754 float can represent all integers up to 2^24 (~16.7 million) correctly
    // should be enough for this use case (comparison in for loop)
    float lightCountVec[4] = { (float)numLights };
    bgfx::setUniform(lightCountVecUniform, lightCountVec);

    bgfx::setBuffer(SAMPLER_START, lightPosBuffer, bgfx::Access::Read);
    bgfx::setBuffer(SAMPLER_START + 1, lightFluxBuffer, bgfx::Access::Read);

    return 0;
}
