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

    // simply a multiplier for the radiance arriving at the camera
    float exposure = 1.0f;

    void move(glm::vec3 delta); // coordinates are in world-space (use forward/up/right to move relative to the camera)
    void rotate(glm::vec2 delta); // rotation around x, y axis
    void zoom(float offset); // > 0 = zoom in (decrease FOV by <offset> angles)

    void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

    glm::vec3 position() const;
    glm::mat4 matrix() const;

    // camera vectors in world-space coordinates
    glm::vec3 forward() const;
    glm::vec3 up() const;
    glm::vec3 right() const;

private:

    static const glm::vec3 X, Y, Z;

    static constexpr float MIN_FOV = 10.0f;
    static constexpr float MAX_FOV = 90.0f;

    glm::vec3 orthUp = Y;

    glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
    glm::quat rotation = glm::identity<glm::quat>();
    glm::quat invRotation = glm::identity<glm::quat>();
};
