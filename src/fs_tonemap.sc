$input v_texcoord0

#include <bgfx_shader.sh>

#define SRGB_CONVERSION_FAST
#include "tonemapping.sh"

SAMPLER2D(s_texColor, 0);

void main()
{
    vec4 result = texture2D(s_texColor, v_texcoord0);

    // exposure
    float exposure = 1.0;
    result *= exposure;

    // tonemapping
    //result.rgb = tonemap_reinhard(result.rgb);
    //result.rgb = tonemap_hable(result.rgb);
    result.rgb = tonemap_duiker(result.rgb);
    //result.rgb = tonemap_aces(result.rgb);
    //result.rgb = tonemap_aces_luminance(result.rgb);

    // gamma correction
    result.rgb = LinearTosRGB(result.rgb);

    gl_FragColor = result;
}
