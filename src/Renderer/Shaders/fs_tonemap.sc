$input v_texcoord0

#include <bgfx_shader.sh>

#include "tonemapping.sh"

uniform vec4 u_exposureVec;
#define u_exposure u_exposureVec.x

uniform vec4 u_tonemappingModeVec;
#define u_tonemappingMode int(u_tonemappingModeVec.x)

#define TONEMAP_NONE 0
#define TONEMAP_EXPONENTIAL 1
#define TONEMAP_REINHARD 2
#define TONEMAP_REINHARD_LUM 3
#define TONEMAP_HABLE 4
#define TONEMAP_DUIKER 5
#define TONEMAP_ACES 6
#define TONEMAP_ACES_LUM 7

SAMPLER2D(s_texColor, 0);

void main()
{
    vec4 result = texture2D(s_texColor, v_texcoord0);
    result.rgb *= u_exposure;

    // this might not be the most performant way on some hardware

    if(u_tonemappingMode == TONEMAP_NONE)
    {
        result.rgb = clamp(result.rgb, 0.0, 1.0);
    }
    else if(u_tonemappingMode == TONEMAP_EXPONENTIAL)
    {
        result.rgb = tonemap_exponential(result.rgb);
    }
    else if(u_tonemappingMode == TONEMAP_REINHARD)
    {
        result.rgb = tonemap_reinhard(result.rgb);
    }
    else if(u_tonemappingMode == TONEMAP_REINHARD_LUM)
    {
        result.rgb = tonemap_reinhard_luminance(result.rgb);
    }
    else if(u_tonemappingMode == TONEMAP_HABLE)
    {
        result.rgb = tonemap_hable(result.rgb);
    }
    else if(u_tonemappingMode == TONEMAP_DUIKER)
    {
        result.rgb = tonemap_duiker(result.rgb);
    }
    else if(u_tonemappingMode == TONEMAP_ACES)
    {
        result.rgb = tonemap_aces(result.rgb);
    }
    else if(u_tonemappingMode == TONEMAP_ACES_LUM)
    {
        result.rgb = tonemap_aces_luminance(result.rgb);
    }

    // gamma correction
    result.rgb = LinearTosRGB(result.rgb);

    gl_FragColor = result;
}
