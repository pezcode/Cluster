#include <bgfx_shader.sh>
#include "samplers.sh"
#include "lights.sh"

// G-Buffer
SAMPLER2D(s_texDiffuseA,          SAMPLER_DEFERRED_DIFFUSE_A);
SAMPLER2D(s_texNormal,            SAMPLER_DEFERRED_NORMAL);
SAMPLER2D(s_texF0Metallic,        SAMPLER_DEFERRED_F0_METALLIC);
SAMPLER2D(s_texEmissiveOcclusion, SAMPLER_DEFERRED_EMISSIVE_OCCLUSION);
SAMPLER2D(s_texDepth,             SAMPLER_DEFERRED_DEPTH);

void main()
{
    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;
    vec3 diffuseColor = texture2D(s_texDiffuseA, texcoord).xyz;
    vec4 emissiveOcclusion = texture2D(s_texEmissiveOcclusion, texcoord);
    vec3 emissive = emissiveOcclusion.xyz;
    float occlusion = emissiveOcclusion.w;

    // TODO
    // directional lights

    vec3 radianceOut = vec3_splat(0.0);
    radianceOut += getAmbientLight().irradiance * diffuseColor * occlusion;
    radianceOut += emissive;

    gl_FragColor = vec4(radianceOut, 1.0);
}
