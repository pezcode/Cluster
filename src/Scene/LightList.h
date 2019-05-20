#pragma once

#include "Scene/Light.h"

#include <vector>

template<typename T>
class LightList
{
public:

    void resize(size_t size);

    std::vector<T> lights;
};

// structures for GPU?
// cull on CPU?
//  new class
