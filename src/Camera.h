#pragma once

#include <glm/vec3.hpp>

struct Camera
{
    glm::vec3 pos = glm::vec3(0.0f, 1.0f, -2.0f);
    glm::vec3 lookAt = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Hor+ field of view scaling
    // fixed vertical fov in degrees (full angle, not half)

    // notes on the chosen value of 73.74:
    // this is 90 degrees horizontal fov on a 4:3 monitor
    // expands to 106.26 at 16:9
    // identical to CS:GO's fixed fov
    float fov = 73.74f;

    float zNear = 0.1f;
    float zFar = 100.0f;
};
