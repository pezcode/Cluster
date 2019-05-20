#include "LightList.h"

#include <glm/gtc/type_ptr.hpp>

bgfx::VertexDecl LightList::Vec3Vertex::decl;

PointLightList::PointLightList() :
    position(nullptr),
    flux(nullptr),
    positionBuffer(BGFX_INVALID_HANDLE),
    fluxBuffer(BGFX_INVALID_HANDLE)
{
}

void PointLightList::init()
{
    positionBuffer = bgfx::createDynamicVertexBuffer(1, Vec3Vertex::decl, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
    fluxBuffer     = bgfx::createDynamicVertexBuffer(1, Vec3Vertex::decl, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
}

void PointLightList::shutdown()
{
    bgfx::destroy(positionBuffer);
    bgfx::destroy(fluxBuffer);
    positionBuffer = fluxBuffer = BGFX_INVALID_HANDLE;

    position.release();
    flux.release();
}

void PointLightList::update()
{
    // TODO bgfx::alloc in here, pass to bgfx, create new fixed vertex buffer

    position = std::make_unique<Vec3Vertex[]>(lights.size());
    flux     = std::make_unique<Vec3Vertex[]>(lights.size());

    for(size_t i = 0; i < lights.size(); i++)
    {
        memcpy(&position[i], glm::value_ptr(lights[i].position), 3 * sizeof(float));
        memcpy(&flux[i],     glm::value_ptr(lights[i].flux),     3 * sizeof(float));
    }

    bgfx::update(positionBuffer, 0, bgfx::makeRef(position.get(), (uint32_t)(sizeof(Vec3Vertex) * lights.size())));
    bgfx::update(fluxBuffer,     0, bgfx::makeRef(flux.get(),     (uint32_t)(sizeof(Vec3Vertex) * lights.size())));
}
