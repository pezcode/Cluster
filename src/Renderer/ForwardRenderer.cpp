#include "ForwardRenderer.h"

#include <bigg.hpp>
#include <bx/string.h>
#include <bx/math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>

ForwardRenderer::ForwardRenderer(const Scene* scene) :
    Renderer(scene),
    program(BGFX_INVALID_HANDLE)
{
}

ForwardRenderer::~ForwardRenderer()
{
}

bool ForwardRenderer::supported()
{
    if(Renderer::supported())
    {
        return true;
    }
    return false;
}

void ForwardRenderer::onInitialize()
{
    baseColorUniform = bgfx::createUniform("u_baseColor", bgfx::UniformType::Vec4);
    metallicRoughnessUniform = bgfx::createUniform("u_metallicRoughness", bgfx::UniformType::Vec4);
    hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
    baseColorSampler = bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler);
    metallicRoughnessSampler = bgfx::createUniform("s_texMetallicRoughness", bgfx::UniformType::Sampler);
    normalSampler = bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler);

    char vsName[128], fsName[128];
    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_forward.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_forward.bin");
    program = bigg::loadProgram(vsName, fsName);
}

void ForwardRenderer::onReset()
{
    
}

void ForwardRenderer::onRender(float dt)
{
    bgfx::ViewId vDefault = 0;

    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);

    bgfx::touch(vDefault); // empty primitive in case nothing follows

    if(!scene->loaded)
        return;
    
    glm::mat4 view = scene->camera.matrix();
    // scale down (default camera far plane is 5)
    float size = glm::compMax(glm::abs(scene->maxBounds - scene->minBounds));
    view = glm::scale(view, glm::vec3(1.0f / size * 5.0f));
    glm::mat4 proj;
    bx::mtxProj(&proj[0][0], scene->camera.fov, float(width) / height,
                scene->camera.zNear, scene->camera.zFar, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(vDefault, &view[0][0], &proj[0][0]);

    for(const Scene::Mesh& mesh : scene->meshes)
    {
        bgfx::setVertexBuffer(0, mesh.vertexBuffer);
        bgfx::setIndexBuffer(mesh.indexBuffer);
        const Scene::Material& mat = scene->materials[mesh.material];
        bindMaterial(mat);
        uint64_t state = BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_ALPHA;
        if(mat.doubleSided)
            state &= ~BGFX_STATE_CULL_MASK;
        bgfx::setState(state);
        bgfx::submit(vDefault, program);
    }
}

void ForwardRenderer::onShutdown()
{
    bgfx::destroy(baseColorUniform);
    bgfx::destroy(metallicRoughnessUniform);
    bgfx::destroy(hasTexturesUniform);
    bgfx::destroy(baseColorSampler);
    bgfx::destroy(metallicRoughnessSampler);
    bgfx::destroy(normalSampler);
    bgfx::destroy(program);
}

void ForwardRenderer::bindMaterial(const Scene::Material& material)
{
    float metallicRoughnessValues[4] = { material.metallic, material.roughness, 0.0f, 0.0f };
    float hasTexturesValues[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
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
}
