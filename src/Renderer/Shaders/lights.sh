#ifndef LIGHTS_SH_HEADER_GUARD
#define LIGHTS_SH_HEADER_GUARD

#include "pbr.sh"

uniform vec4 u_lightCountVec;
#define u_pointLightCount uint(u_lightCountVec.x)

// actually vec3, but with padding
// first three samplers are taken by PBR texture samplers
// GLSL compiler doesn't like non-constants here
BUFFER_RO(b_pointLightPosition, vec4, 3);
BUFFER_RO(b_pointLightPower, vec4, 4);

#define LIGHTS_SAMPLER_END (PBR_SAMPLER_END + 2)

struct PointLight
{
    vec3 position;
    vec3 intensity;
    float radius;
};

// primary source:
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
// also really good:
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

#define INV_4PI 0.07957747154

float distanceAttenuation(float distance)
{
    // only for point lights

    // physics: inverse square falloff
    // pretend point lights are tiny spheres of 1 cm radius
    return 1.0 / max(distance * distance, 0.01 * 0.01);
}

float smoothAttenuation(float distance, float radius)
{
    // window function with smooth transition to 0
    // radius is arbitrary
    float nom = clamp(1.0 - pow(distance / radius, 4.0), 0.0, 1.0);
    return nom * nom * distanceAttenuation(distance);
}

float calculateLightRadius(vec3 position, vec3 intensity)
{
    const float MAX_RADIUS = 10.0;
    return MAX_RADIUS;
}

uint pointLightCount()
{
    return u_pointLightCount;
}

PointLight getPointLight(uint i)
{
    PointLight light;
    light.position = b_pointLightPosition[i].xyz;
    // scale by area of unit sphere
    light.intensity = b_pointLightPower[i].xyz * INV_4PI;
    light.radius = calculateLightRadius(light.position, light.intensity);
    return light;
}

#endif // LIGHTS_SH_HEADER_GUARD
