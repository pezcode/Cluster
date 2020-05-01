#include "Light.h"

#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/component_wise.hpp>

float PointLight::calculateRadius() const
{
    // radius = where attenuation would lead to an intensity of 1W/m^2
    const float INTENSITY_CUTOFF = 1.0f;
    const float ATTENTUATION_CUTOFF = 0.05f;
    glm::vec3 intensity = flux / (4.0f * glm::pi<float>());
    float maxIntensity = glm::compMax(intensity);
    float attenuation = glm::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;
    return 1.0f / sqrtf(attenuation);
}
