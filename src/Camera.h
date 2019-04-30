#pragma once

#include <glm/vec3.hpp>

class Camera
{
public:

    Camera();

    // Hor+ field of view scaling
    // fixed vertical fov in degrees
    float fov;

    glm::vec3 position;
};
