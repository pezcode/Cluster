#include "ClusteredRenderer.h"

ClusteredRenderer::ClusteredRenderer(const Scene* scene) :
    Renderer(scene)
{
}

ClusteredRenderer::~ClusteredRenderer()
{
}

void ClusteredRenderer::initialize()
{
    Renderer::initialize();
}

void ClusteredRenderer::reset(uint16_t width, uint16_t height)
{
    Renderer::reset(width, height);
}

void ClusteredRenderer::render(float dt)
{
}

void ClusteredRenderer::shutdown()
{
    Renderer::shutdown();
}
