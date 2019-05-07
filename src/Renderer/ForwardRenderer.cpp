#include "ForwardRenderer.h"

#include <bigg.hpp>
#include <bx/string.h>
#include <bx/math.h>
#include <glm/gtc/matrix_transform.hpp>

ForwardRenderer::ForwardRenderer(const Scene* scene) :
    Renderer(scene),
    program(BGFX_INVALID_HANDLE),
    mTime(0.0f)
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
    char vsName[128], fsName[128];
    bx::snprintf(vsName, BX_COUNTOF(vsName), "%s%s", shaderDir(), "vs_forward.bin");
    bx::snprintf(fsName, BX_COUNTOF(fsName), "%s%s", shaderDir(), "fs_forward.bin");
    program = bigg::loadProgram(vsName, fsName);

    baseColorSampler = bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler);
    metallicRoughnessSampler = bgfx::createUniform("s_texMetallicRoughness", bgfx::UniformType::Sampler);
}

void ForwardRenderer::onReset()
{
    
}

void ForwardRenderer::onRender(float dt)
{
    bgfx::ViewId vDefault = 0;

    mTime += dt;

    if(!scene->loaded)
        return;
    
    glm::mat4 view = scene->camera.matrix();
    glm::mat4 proj;
    bx::mtxProj(&proj[0][0], scene->camera.fov, float(width) / height,
                scene->camera.zNear, scene->camera.zFar, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(vDefault, &view[0][0], &proj[0][0]);

    bgfx::setViewClear(vDefault, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030FF, 1.0f, 0);
    bgfx::setViewRect(vDefault, 0, 0, width, height);
    bgfx::setViewFrameBuffer(vDefault, frameBuffer);

    bgfx::touch(vDefault); // empty primitive in case nothing follows

    for(const Scene::Mesh& mesh : scene->meshes)
    {
        const Scene::Material& mat = scene->materials[mesh.material];
        glm::mat4 mtx;
        mtx = glm::rotate(mtx, mTime * 5.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        //bgfx::setTransform(&mtx[0][0]);
        bgfx::setVertexBuffer(0, mesh.vertexBuffer);
        bgfx::setIndexBuffer(mesh.indexBuffer);
        bgfx::setTexture(0, baseColorSampler, mat.baseColor);
        bgfx::setTexture(1, metallicRoughnessSampler, mat.metallicRoughness);
        bgfx::setState(BGFX_STATE_DEFAULT);
        bgfx::submit(vDefault, program);
    }
}

void ForwardRenderer::onShutdown()
{
    bgfx::destroy(baseColorSampler);
    bgfx::destroy(metallicRoughnessSampler);
    bgfx::destroy(program);
}
