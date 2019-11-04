#include <bgfx_shader.sh>
#include "samplers.sh"
#include "lights.sh"

// G-Buffer
SAMPLER2D(s_texDiffuseA, SAMPLER_DEFERRED_DIFFUSE_A);

void main()
{
    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;
    vec3 albedo = texture2D(s_texDiffuseA, texcoord).xyz;

    AmbientLight light = getAmbientLight();
    vec3 radianceOut = albedo * light.irradiance;

    gl_FragColor = vec4(radianceOut, 1.0);
}
