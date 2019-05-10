#include "Camera.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

const glm::vec3 Camera::X = { 1.0f, 0.0f, 0.0f };
const glm::vec3 Camera::Y = { 0.0f, 1.0f, 0.0f };
const glm::vec3 Camera::Z = { 0.0f, 0.0f, 1.0f };

void Camera::move(glm::vec3 delta)
{
    position += delta;
}

void Camera::rotate(glm::vec2 delta)
{
    // TODO lock z-axis
    // TODO limit x-rotation to 89 degrees up or down
    delta = glm::radians(delta);
    // limit pitch
    //float pitch = glm::angle(orthUp, up());
    //if((pitch + delta.x) > MAX_PITCH)
    //    delta.x = 0.0f;
    //delta.x = glm::clamp(oldPitch + delta.x, 0.0f, MAX_PITCH) - oldPitch;

    rotation = glm::rotate(glm::quat(), delta.y, Y) * // yaw
               glm::rotate(glm::quat(), delta.x, X) * // pitch
               rotation;
    // lock z-rotation
    //float roll = glm::angle(orthUp, up());
    //float roll = glm::roll(rotation);
    //rotation = glm::rotate(glm::quat(), -roll, Z) * rotation;
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
    rotation = glm::rotation(forward, Z);

    // correct the up vector
    glm::vec3 right = glm::cross(glm::normalize(up), forward); // left-handed coordinate system
    /*glm::vec3*/ orthUp = -glm::cross(right, forward);
    glm::quat upRotation = glm::rotation(rotation * orthUp, Y);
    rotation = upRotation * rotation;

    // inverse of the model rotation
    // maps camera space vectors to model vectors
    invRotation = glm::conjugate(rotation);
}

const glm::mat4 Camera::matrix() const
{
    return glm::toMat4(rotation) * glm::translate(glm::mat4(), -position);
}

glm::vec3 Camera::forward() const
{
    return invRotation * Z;
}

glm::vec3 Camera::up() const
{
    return invRotation * Y;
}

glm::vec3 Camera::right() const
{
    return invRotation * X;
}
