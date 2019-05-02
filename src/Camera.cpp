#include "Camera.h"

Camera::Camera()
{
    // 90 degrees horizontal fov on a 4:3 monitor
    // expands to 106.26 at 16:9
    // identical to CS:GO's fixed fov
    fov = 73.74f;
}

Camera::~Camera()
{
}
