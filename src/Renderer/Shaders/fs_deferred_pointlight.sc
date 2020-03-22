#include "common.sh"
#include <bgfx_shader.sh>
#include "samplers.sh"
#include "pbr.sh"
#include "lights.sh"
#include "util.sh"

// G-Buffer
SAMPLER2D(s_texDiffuseA,          SAMPLER_DEFERRED_DIFFUSE_A);
SAMPLER2D(s_texNormal,            SAMPLER_DEFERRED_NORMAL);
SAMPLER2D(s_texF0Metallic,        SAMPLER_DEFERRED_F0_METALLIC);
SAMPLER2D(s_texDepth,             SAMPLER_DEFERRED_DEPTH);

uniform vec4 u_lightIndexVec;
#define u_lightIndex uint(u_lightIndexVec.x)

void main()
{
    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;
    
    vec4 diffuseA = texture2D(s_texDiffuseA, texcoord);
    vec3 N = unpackNormal(texture2D(s_texNormal, texcoord).xy);
    vec4 F0Metallic = texture2D(s_texF0Metallic, texcoord);

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

    // lighting

    vec3 radianceOut = vec3_splat(0.0);

    PointLight light = getPointLight(u_lightIndex);
    light.position = mul(u_view, vec4(light.position, 1.0)).xyz;

    
    float dist = distance(light.position, fragPos);
    float attenuation = smoothAttenuation(dist, light.radius);
    if(attenuation > 0.0)
    {
        vec3 V = normalize(-fragPos);
        vec3 L = normalize(light.position - fragPos);
        vec3 radianceIn = light.intensity * attenuation;
        float NoL = saturate(dot(N, L));
        radianceOut += BRDF(V, L, N, mat) * radianceIn * NoL;
    }

    gl_FragColor = vec4(radianceOut, 1.0);
}
