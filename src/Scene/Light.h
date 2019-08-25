#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct PointLight
{
    glm::vec3 position;
    // spectral power (aka flux) in W
    // radiometric value (ie. linear physical value), not photometric (based on human eye sensitivity)
    // TODO does this make sense? shouldn't this be photometric
    glm::vec3 power;
};

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 power;
};

struct SpotLight
{
    glm::vec3 position;
    glm::vec3 direction;
    float angle; // full angle in degrees
    glm::vec3 power;
};

struct AmbientLight
{
    glm::vec3 irradiance;
};
