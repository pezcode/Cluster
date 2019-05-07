#include "Camera.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

void Camera::move(glm::vec3 delta)
{
    glm::vec3 camDelta = /*rotationMat **/ glm::vec4(delta, 0.0f);
    pos += camDelta;
    update();
}

void Camera::rotate(glm::vec3 delta)
{
    angles += delta;
    angles = glm::mod(angles, glm::pi<float>() * 2.0f);
    update();
}

const glm::mat4& Camera::matrix() const
{
    return mat;
}

void Camera::setMatrix(const glm::mat4& newMat)
{
    decompose(newMat);
    update();
}

void Camera::update()
{
    rotationMat = glm::yawPitchRoll(angles.y, angles.x, 0.0f);
    mat = glm::translate(glm::mat4(), pos) * rotationMat;
}

void Camera::decompose(const glm::mat4& transformation)
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(transformation, scale, rotation, translation, skew, perspective);

    pos = translation;
    angles = glm::eulerAngles(rotation);
}
