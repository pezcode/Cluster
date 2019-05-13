$input v_worldpos, v_normal, v_tangent, v_bitangent, v_texcoord0

#include <bgfx_shader.sh>
#include "pbr_shader.sh"

void main()
{
    // why can't these be global?
    const vec3 lightPos = vec3(0.5, 0.5, 0.5);
    const vec3 lightFlux = vec3(30.0, 20.0, 20.0); // RGB

    const vec3 ambientLightFlux = vec3(0.05, 0.05, 0.05);

    PBRMaterial mat = pbrMaterial(v_texcoord0);

    // do shading

    vec3 camPos = mul(u_invView, vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    // normal map is in tangent space
    // convert to world space
    mat3 TBN = mat3(v_tangent, v_bitangent, v_normal);
    mat3 invTBN = transpose(TBN);
    vec3 normalOffset = mul(invTBN, mat.normal);

    vec3 N = normalize(v_normal + normalOffset);
    vec3 V = normalize(camPos - v_worldpos);

    vec3 radianceOut = vec3(0.0, 0.0, 0.0);
    // for each light:
    {
        vec3 L = normalize(lightPos - v_worldpos);
        float distance    = length(lightPos - v_worldpos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radianceIn   = lightFlux * attenuation; 
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        radianceOut += BRDF(V, L, N, mat) * radianceIn * NoL;
    }

    vec3 ambient = ambientLightFlux * mat.albedo.rgb; // * ambientOcclusion
    vec3 color = radianceOut + ambient;
    
    // output goes straight to HDR framebuffer, no clamping
    // tonemapping happens in final blit

    gl_FragColor.rgb = color.rgb;
    gl_FragColor.a = mat.albedo.a;
}
