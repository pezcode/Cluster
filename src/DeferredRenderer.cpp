#include "DeferredRenderer.h"

DeferredRenderer::DeferredRenderer(const Scene* scene) :
    Renderer(scene),
    bufferList{ { 0, "Albedo" }, { 0, "Normal" }, { 0, "Specular" }, { 0, nullptr } }
{
    buffers = bufferList;
}

void DeferredRenderer::render(float dt)
{

}
