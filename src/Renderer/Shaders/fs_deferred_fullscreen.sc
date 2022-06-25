#include <bgfx_shader.sh>
#include "samplers.sh"
#include "pbr.sh"
#include "lights.sh"
#include "util.sh"

// G-Buffer
SAMPLER2D(s_texDiffuseA,          SAMPLER_DEFERRED_DIFFUSE_A);
SAMPLER2D(s_texNormal,            SAMPLER_DEFERRED_NORMAL);
SAMPLER2D(s_texF0Metallic,        SAMPLER_DEFERRED_F0_METALLIC);
SAMPLER2D(s_texEmissiveOcclusion, SAMPLER_DEFERRED_EMISSIVE_OCCLUSION);
SAMPLER2D(s_texDepth,             SAMPLER_DEFERRED_DEPTH);

void main()
{
    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;

    vec4 diffuseA = texture2D(s_texDiffuseA, texcoord);
    vec3 N = unpackNormal(texture2D(s_texNormal, texcoord).xy);
    vec4 F0Metallic = texture2D(s_texF0Metallic, texcoord);
    vec4 emissiveOcclusion = texture2D(s_texEmissiveOcclusion, texcoord);
    vec3 emissive = emissiveOcclusion.xyz;
    float occlusion = emissiveOcclusion.w;

    // unpack material parameters used by the PBR BRDF function
    PBRMaterial mat;
    mat.diffuseColor = diffuseA.xyz;
    mat.a = diffuseA.w;
    mat.F0 = F0Metallic.xyz;
    mat.metallic = F0Metallic.w;

    // get fragment position
    // rendering happens in view space
    vec4 screen = gl_FragCoord;
    screen.z = texture2D(s_texDepth, texcoord).x;
    vec3 fragPos = screen2Eye(screen).xyz;

    vec3 radianceOut = vec3_splat(0.0);

    // directional light
    vec3 V = normalize(-fragPos);
    float NoV = abs(dot(N, V)) + 1e-5;
    vec3 msFactor = multipleScatteringFactor(mat, NoV);

    SunLight light = getSunLight();
    vec3 L = -mul(u_view, vec4(light.direction, 0.0)).xyz;
    float NoL = saturate(dot(N, L));
    radianceOut += BRDF(V, L, N, NoV, NoL, mat) * msFactor * light.radiance * NoL;

    // ambient lighting
    radianceOut += getAmbientLight().irradiance * mat.diffuseColor * occlusion;
    radianceOut += emissive;

    gl_FragColor = vec4(radianceOut, 1.0);
}
