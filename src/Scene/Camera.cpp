#include "Camera.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 Camera::X = { 1.0f, 0.0f, 0.0f };
const glm::vec3 Camera::Y = { 0.0f, 1.0f, 0.0f };
const glm::vec3 Camera::Z = { 0.0f, 0.0f, 1.0f };

void Camera::move(glm::vec3 delta)
{
    pos += delta;
}

void Camera::rotate(glm::vec2 delta)
{
    delta = glm::radians(delta);

    // limit pitch
    float dot = glm::dot(upAxis, forward());
    if((dot < -0.99f && delta.x < 0.0f) || // angle nearing 180 degrees
       (dot > 0.99f && delta.x > 0.0f))    // angle nearing 0 degrees
        delta.x = 0.0f;

    // pitch is relative to current sideways rotation
    // yaw happens independently
    // this prevents roll
    rotation = glm::rotate(glm::identity<glm::quat>(), delta.x, X) *           // pitch
               rotation * glm::rotate(glm::identity<glm::quat>(), delta.y, Y); // yaw
    // normalize?
    invRotation = glm::conjugate(rotation);
}

void Camera::zoom(float offset)
{
    fov = glm::clamp(fov - offset, MIN_FOV, MAX_FOV);
}

void Camera::lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
    pos = position;
    upAxis = up;

    // model rotation
    // maps vectors to camera space (x, y, z)
    glm::vec3 forward = glm::normalize(target - position);
    rotation = glm::rotation(forward, Z);

    // correct the up vector
    // the cross product of non-orthogonal vectors is not normalized
    glm::vec3 right = glm::normalize(glm::cross(glm::normalize(up), forward)); // left-handed coordinate system
    glm::vec3 orthUp = glm::cross(forward, right);
    glm::quat upRotation = glm::rotation(rotation * orthUp, Y);
    rotation = upRotation * rotation;

    // inverse of the model rotation
    // maps camera space vectors to model vectors
    invRotation = glm::conjugate(rotation);
}

glm::vec3 Camera::position() const
{
    return pos;
}

glm::mat4 Camera::matrix() const
{
    return glm::toMat4(rotation) * glm::translate(glm::identity<glm::mat4>(), -pos);
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
