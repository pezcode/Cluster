#include <bgfx_shader.sh>
#include "samplers.sh"
#include "pbr.sh"
#include "lights.sh"
#include "util.sh"

// G-Buffer
SAMPLER2D(s_texDiffuseA,   SAMPLER_DEFERRED_DIFFUSE_A);
SAMPLER2D(s_texNormal,     SAMPLER_DEFERRED_NORMAL);
SAMPLER2D(s_texF0Metallic, SAMPLER_DEFERRED_F0_METALLIC);
SAMPLER2D(s_texDepth,      SAMPLER_DEFERRED_DEPTH);

uniform vec4 u_camPos;

uniform vec4 u_lightIndexVec;
#define u_lightIndex uint(u_lightIndexVec.x)

void main()
{
    vec2 texcoords = gl_FragCoord.xy / u_viewRect.zw;
    
    vec4 diffuseA = texture2D(s_texDiffuseA, texcoords);
    vec3 N = texture2D(s_texNormal, texcoords).xyz * 2.0 - 1.0;
    vec4 F0Metallic = texture2D(s_texF0Metallic, texcoords);

    // unpack material parameters used by the PBR BRDF function
    PBRMaterial mat;
    mat.diffuseColor = diffuseA.xyz;
    mat.a = diffuseA.w;
    mat.F0 = F0Metallic.xyz;
    mat.metallic = F0Metallic.w;

    // get fragment world position
    vec4 screen = gl_FragCoord;
    screen.z = texture2D(s_texDepth, texcoords).x;
    vec4 eyePos = screen2Eye(screen);
    vec3 fragPos = mul(u_invView, eyePos).xyz;

    vec3 camPos = u_camPos.xyz;
    vec3 V = normalize(camPos - fragPos);

    // lighting

    PointLight light = getPointLight(u_lightIndex);

    vec3 radianceOut = vec3_splat(0.0);
    float dist = distance(light.position, fragPos);
    if(dist < light.radius)
    {
        float attenuation = smoothAttenuation(dist, light.radius);
        vec3 L = normalize(light.position - fragPos);
        vec3 radianceIn = light.intensity * attenuation;
        float NoL = saturate(dot(N, L));
        radianceOut = BRDF(V, L, N, mat) * radianceIn * NoL;
    }

    //radianceOut = vec3(0.2, 0.0, 0.0);

    gl_FragColor = vec4(radianceOut, 1.0);
}
