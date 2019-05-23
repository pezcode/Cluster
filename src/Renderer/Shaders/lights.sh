#ifndef LIGHTS_SH_HEADER_GUARD
#define LIGHTS_SH_HEADER_GUARD

#include "pbr.sh"

uniform vec4 u_lightCountVec;
#define u_pointLightCount uint(u_lightCountVec.x)

// actually vec3, but with padding
// first three are taken by PBR texture samplers
// OpenGL compiler doesn't like non-constants here
BUFFER_RO(b_pointLightPosition, vec4, 3);
BUFFER_RO(b_pointLightpower, vec4, 4);

#define LIGHTS_SAMPLER_END (PBR_SAMPLER_END + 2)

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

uint pointLightCount()
{
    return u_pointLightCount;
}

vec3 pointLightPosition(uint i)
{
    return b_pointLightPosition[i].xyz;
}

vec3 pointLightIntensity(uint i)
{
    return b_pointLightpower[i].xyz * INV_4PI;
}

/*
vec3 shadePointLight(vec3 N, vec3 L, vec3 eye, vec3 pos, vec3 power, PBRMaterial mat)
{

}
*/

#endif // LIGHTS_SH_HEADER_GUARD
