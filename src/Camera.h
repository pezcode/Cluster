#pragma once

#include <glm/glm.hpp>

struct Camera
{
    // fixed vertical fov (Hor+) in degrees (full angle, not half)
    // notes on 73.74:
    // this is 90 degrees horizontal fov on a 4:3 monitor
    // expands to 106.26 at 16:9
    float fov = 73.74f;

    float zNear = 0.01f;
    float zFar = 100.0f;

    void move(glm::vec3 delta); // relative to camera forward
    void rotate(glm::vec3 delta); // x, y, z axis (rotation order: y -> x -> z)

    const glm::mat4& matrix() const;
    void setMatrix(const glm::mat4& newMat);

private:

    void update(); // calculate matrix and rotationMatrix
    void decompose(const glm::mat4& transformation); // extract position and angles

    glm::vec3 pos = { 0.0f, 1.0f, -2.0f };
    glm::vec3 angles = { 0.0f, 0.0f, 0.0f };

    glm::mat4 mat;
    glm::mat4 rotationMat;
};
