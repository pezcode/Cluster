#include "LightList.h"

#include <glm/gtc/type_ptr.hpp>

// initialized in Scene::init
bgfx::VertexDecl LightList::Vec4Vertex::decl;

PointLightList::PointLightList() :
    positionBuffer(BGFX_INVALID_HANDLE),
    powerBuffer(BGFX_INVALID_HANDLE)
{
}

void PointLightList::init()
{

}

void PointLightList::shutdown()
{
    if(bgfx::isValid(positionBuffer))
        bgfx::destroy(positionBuffer);
    if(bgfx::isValid(powerBuffer))
        bgfx::destroy(powerBuffer);
    positionBuffer = powerBuffer = BGFX_INVALID_HANDLE;
}

void PointLightList::update()
{
    // is memory cleared to 0?
    const bgfx::Memory* memPosition = bgfx::alloc(uint32_t(sizeof(Vec4Vertex) * lights.size()));
    const bgfx::Memory* memPower    = bgfx::alloc(uint32_t(sizeof(Vec4Vertex) * lights.size()));

    uint8_t* position = memPosition->data;
    uint8_t* power    = memPower->data;

    size_t stride = Vec4Vertex::decl.getStride();
    size_t filled = 3 * sizeof(float);

    for(size_t i = 0; i < lights.size(); i++)
    {
        size_t offset = i * stride;
        memcpy(&position[offset], glm::value_ptr(lights[i].position), filled);
        memcpy(&power[offset],    glm::value_ptr(lights[i].power),    filled);
    }

    // initially this used dynamic vertex buffers
    // but apparently they normalize input regardless of decl and often fail to send data
    positionBuffer = bgfx::createVertexBuffer(memPosition, Vec4Vertex::decl, BGFX_BUFFER_COMPUTE_READ);
    powerBuffer    = bgfx::createVertexBuffer(memPower,    Vec4Vertex::decl, BGFX_BUFFER_COMPUTE_READ);
}
