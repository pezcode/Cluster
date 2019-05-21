#ifndef LIGHTS_SH_HEADER_GUARD
#define LIGHTS_SH_HEADER_GUARD

#include "pbr.sh"

uniform vec4 u_lightCountVec;
#define u_pointLightCount uint(u_lightCountVec.x)

// actually vec3, but with padding
// first three are taken by PBR texture samplers
// OpenGL compiler doesn't like non-constants here
BUFFER_RO(b_pointLightPosition, vec4, 3);
BUFFER_RO(b_pointLightFlux, vec4, 4);

#define LIGHTS_SAMPLER_END (PBR_SAMPLER_END + 2)

uint pointLightCount()
{
    return u_pointLightCount;
}

vec3 pointLightPosition(uint i)
{
    return b_pointLightPosition[i].xyz;
}

vec3 pointLightFlux(uint i)
{
    return b_pointLightFlux[i].xyz;
}

/*
vec3 shadePointLight(vec3 N, vec3 L, vec3 eye, vec3 pos, vec3 flux, PBRMaterial mat)
{

}
*/

#endif // LIGHTS_SH_HEADER_GUARD
