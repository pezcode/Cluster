#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct PointLight
{
    glm::vec3 position;
    glm::vec3 flux; // RGB
};

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 flux;
};

struct SpotLight
{
    glm::vec3 position;
    glm::vec3 direction;
    float angle; // full angle in degrees
    glm::vec3 flux; // RGB
};

struct AmbientLight
{
    glm::vec3 flux; // RGB
};
