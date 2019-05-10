#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Camera
{
    // fixed vertical field of view (Hor+) in degrees (full angle, not half)
    // default value is 90 degrees horizontal FOV on a 4:3 monitor
    // expands to 106.26 at 16:9
    // taken from Counter-Strike Global Offensive
    float fov = 73.7397953f; // degrees(atan(tan(radians(90)/2) / (4/3)) * 2)

    // the ratio zFar/zNear should be minimal
    // the higher it is, the more precision loss we get in the depth buffer (-> z-fighting)
    float zNear = 0.1f; // projection plane
    float zFar = 5.0f; // far plane

    void move(glm::vec3 delta); // camera-space
    void rotate(glm::vec2 delta); // rotation around x, y axis in camera-space (rotation order: y [yaw] -> x [pitch])
    void zoom(float offset); // > 0 = zoom in (decrease FOV by <offset> angles)

    void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

    const glm::mat4 matrix() const;

    glm::vec3 forward() const;
    glm::vec3 up() const;
    glm::vec3 right() const;

private:

    static const glm::vec3 X, Y, Z;

    static constexpr float MIN_FOV = 10.0f;
    static constexpr float MAX_FOV = 90.0f;

    //static constexpr float MIN_PITCH = glm::radians(1.0f);
    //static constexpr float MAX_PITCH = glm::radians(89.0f);

    glm::vec3 orthUp;

    glm::vec3 position;
    glm::quat rotation;
    glm::quat invRotation;
};
