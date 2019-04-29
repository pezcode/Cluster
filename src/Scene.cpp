#include "Scene.h"

bgfx::VertexDecl PosColorVertex::ms_decl;

Scene::Scene() :
    s_cubeVertices{
        { -1.0f, 1.0f, 1.0f, 0xff000000 },   { 1.0f, 1.0f, 1.0f, 0xff0000ff },   { -1.0f, -1.0f, 1.0f, 0xff00ff00 },
        { 1.0f, -1.0f, 1.0f, 0xff00ffff },   { -1.0f, 1.0f, -1.0f, 0xffff0000 }, { 1.0f, 1.0f, -1.0f, 0xffff00ff },
        { -1.0f, -1.0f, -1.0f, 0xffffff00 }, { 1.0f, -1.0f, -1.0f, 0xffffffff }
    },
    s_cubeTriList {
        2, 1, 0, 2, 3, 1, 5, 6, 4, 7, 6, 5, 4, 2, 0, 6, 2, 4,
        3, 5, 1, 3, 7, 5, 1, 4, 0, 1, 5, 4, 6, 3, 2, 7, 3, 6
    }
{
    //std::run_once();
}
