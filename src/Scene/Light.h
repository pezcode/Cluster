#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct PointLight
{
    glm::vec3 position;
    glm::vec4 color;
};

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec4 color;
};

struct SpotLight : PointLight
{
    glm::vec3 direction;
    float angle; // full angle in degrees
};
