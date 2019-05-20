#ifndef LIGHTS_SH_HEADER_GUARD
#define LIGHTS_SH_HEADER_GUARD

#include "pbr.sh"

uniform vec4 u_lightCountVec;
#define u_pointLightCount floor(u_lightCountVec.x)

// first three are taken by PBR texture samplers
BUFFER_RO(b_pointLightPosition, vec3, (PBR_SAMPLER_END + 1));
BUFFER_RO(b_pointLightFlux, vec3, (PBR_SAMPLER_END + 2));

#define LIGHTS_SAMPLER_END (PBR_SAMPLER_END + 2)

uint pointLightCount()
{
    return u_pointLightCount;
}

vec3 pointLightPosition(uint i)
{
    return b_pointLightPosition[i];
}

vec3 pointLightFlux(uint i)
{
    return b_pointLightFlux[i];
}

/*
vec3 shadePointLight(vec3 N, vec3 L, vec3 eye, vec3 pos, vec3 flux, PBRMaterial mat)
{

}
*/

#endif // LIGHTS_SH_HEADER_GUARD
