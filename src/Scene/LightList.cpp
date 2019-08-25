#include "LightList.h"

#include <glm/gtc/type_ptr.hpp>

bgfx::VertexDecl LightList::PointLightVertex::decl;

PointLightList::PointLightList() : buffer(BGFX_INVALID_HANDLE)
{
}

void PointLightList::init()
{
    LightList::PointLightVertex::init();
    buffer = bgfx::createDynamicVertexBuffer(1, PointLightVertex::decl, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
}

void PointLightList::shutdown()
{
    bgfx::destroy(buffer);
    buffer = BGFX_INVALID_HANDLE;
}

void PointLightList::update()
{
    size_t stride = PointLightVertex::decl.getStride();
    const bgfx::Memory* mem = bgfx::alloc(uint32_t(stride * lights.size()));

    for(size_t i = 0; i < lights.size(); i++)
    {
        PointLightVertex* light = (PointLightVertex*)(mem->data + (i * stride));
        light->position = lights[i].position;
        light->power = lights[i].power;
        light->radius = lights[i].calculateRadius();
    }

    bgfx::update(buffer, 0, mem);
}
