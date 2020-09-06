#include "LightList.h"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

bgfx::VertexLayout LightList::PointLightVertex::layout;

void PointLightList::init()
{
    LightList::PointLightVertex::init();
    buffer =
        bgfx::createDynamicVertexBuffer(1, PointLightVertex::layout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
}

void PointLightList::shutdown()
{
    bgfx::destroy(buffer);
    buffer = BGFX_INVALID_HANDLE;
}

void PointLightList::update()
{
    size_t stride = PointLightVertex::layout.getStride();
    const bgfx::Memory* mem = bgfx::alloc(uint32_t(stride * std::max(lights.size(), (size_t)1)));

    for(size_t i = 0; i < lights.size(); i++)
    {
        PointLightVertex* light = (PointLightVertex*)(mem->data + (i * stride));
        light->position = lights[i].position;
        // intensity = flux per unit solid angle (steradian)
        // there are 4*pi steradians in a sphere
        light->intensity = lights[i].flux / (4.0f * glm::pi<float>());
        light->radius = lights[i].calculateRadius();
    }

    bgfx::update(buffer, 0, mem);
}
