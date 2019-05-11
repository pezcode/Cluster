#pragma once

#include <bgfx/bgfx.h>

struct Mesh
{
    bgfx::VertexBufferHandle vertexBuffer = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle indexBuffer = BGFX_INVALID_HANDLE;
    unsigned int material = 0; // index into materials vector
};
