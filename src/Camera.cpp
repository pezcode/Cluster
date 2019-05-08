#include "Camera.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

void Camera::move(glm::vec3 delta)
{
    position += delta;
}

void Camera::rotate(glm::vec2 delta)
{
    // TODO limit total x rotation
    rotation = glm::rotate(glm::quat(), glm::radians(delta.y), { 0.0f, 1.0f, 0.0f }) *
               glm::rotate(glm::quat(), glm::radians(delta.x), { 1.0f, 0.0f, 0.0f }) *
               rotation;
    invRotation = glm::conjugate(rotation);
}

void Camera::zoom(float offset)
{
    fov = glm::clamp(fov - offset, MIN_FOV, MAX_FOV);
}

void Camera::lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
    this->position = position;

    // model rotation
    // maps vectors to camera space (x, y, z)
    glm::vec3 forward = glm::normalize(target - position);
    rotation = glm::rotation(forward, { 0.0f, 0.0f, 1.0f });

    // correct the up vector
    glm::vec3 right = glm::cross(glm::normalize(up), forward); // left-handed coordinate system
    glm::vec3 orthUp = -glm::cross(right, forward);
    glm::quat upRotation = glm::rotation(rotation * orthUp, { 0.0f, 1.0f, 0.0f });
    rotation = upRotation * rotation;

    // inverse of the model rotation
    // maps camera space vectors to model vectors
    invRotation = glm::conjugate(rotation);
}

const glm::mat4 Camera::matrix() const
{
    return glm::mat4_cast(rotation) * glm::translate(glm::mat4(), -position);
}

glm::vec3 Camera::forward() const
{
    return invRotation * glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::vec3 Camera::up() const
{
    return invRotation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Camera::right() const
{
    return invRotation * glm::vec3(1.0f, 0.0f, 0.0f);
}
