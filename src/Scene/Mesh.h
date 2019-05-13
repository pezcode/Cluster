#pragma once

#include <bgfx/bgfx.h>
#include <mutex>

struct Mesh
{
    bgfx::VertexBufferHandle vertexBuffer = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle indexBuffer = BGFX_INVALID_HANDLE;
    unsigned int material = 0; // index into materials vector

    // bgfx vertex attributes
    // initialized by Scene
    struct PosNormalTangentTex0Vertex
    {
        float x, y, z;    // position
        float nx, ny, nz; // normal
        float tx, ty, tz; // tangent
        float bx, by, bz; // bitangent
        float u, v;       // UV coordinates

        static void init()
        {
            decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();
        }
        static bgfx::VertexDecl decl;
        static std::once_flag initFlag;
    };
};
