#ifndef LIGHTS_SH_HEADER_GUARD
#define LIGHTS_SH_HEADER_GUARD

#include "samplers.sh"

uniform vec4 u_lightCountVec;
#define u_pointLightCount uint(u_lightCountVec.x)

uniform vec4 u_ambientLightIrradiance;

// for each light:
//   vec4 position (w is padding)
//   vec4 power + radius (xyz is power, w is radius)
BUFFER_RO(b_pointLights, vec4, SAMPLER_LIGHTS_POINTLIGHTS);

struct PointLight
{
    vec3 position;
    vec3 intensity;
    float radius;
};

struct AmbientLight
{
    vec3 irradiance;
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
    float nom = saturate(1.0 - pow(distance / radius, 4.0));
    return nom * nom * distanceAttenuation(distance);
}

uint pointLightCount()
{
    return u_pointLightCount;
}

PointLight getPointLight(uint i)
{
    PointLight light;
    light.position = b_pointLights[2 * i + 0].xyz;
    vec4 powerRadiusVec = b_pointLights[2 * i + 1];
    light.intensity = powerRadiusVec.xyz * INV_4PI;
    light.radius = powerRadiusVec.w;
    return light;
}

AmbientLight getAmbientLight()
{
    AmbientLight light;
    light.irradiance = u_ambientLightIrradiance.xyz;
    return light;
}

#endif // LIGHTS_SH_HEADER_GUARD
