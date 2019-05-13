$input v_worldpos, v_normal, v_tangent, v_texcoord0

#include <bgfx_shader.sh>

#define SRGB_CONVERSION_FAST
#include "tonemapping.sh"
#include "pbr_shader.sh"



void main()
{
    // why can't these be global?
    const vec3 lightPos = vec3(0.5, 0.5, 0.5);
    const vec3 lightFlux = vec3(100.0, 100.0, 100.0); // RGB

    const vec3 ambientLightFlux = vec3(0.1, 0.1, 0.1);

    PBRMaterial mat = pbrMaterial(v_texcoord0);

    vec3 worldPos = v_worldpos;
    vec3 camPos = mul(u_invView, vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    
    // do shading

    vec3 N = normalize(v_normal); 
    vec3 V = normalize(camPos - worldPos);

    vec3 ambient = ambientLightFlux * mat.albedo.rgb; // * ambientOcclusion
    vec3 radianceOut = vec3(0.0, 0.0, 0.0);

    // for each light:
    {
        vec3 L = normalize(lightPos - worldPos);
        float distance    = length(lightPos - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radianceIn   = lightFlux * attenuation; 

        float NoL = clamp(dot(N, L), 0.0, 1.0);
        radianceOut += BRDF(V, L, N, mat) * radianceIn * NoL;
    }

    vec3 color = radianceOut + ambient;
    
    // output goes straight to HDR framebuffer, no clamping
    // tonemapping happens in final blit

    gl_FragColor.rgb = color.rgb;
    gl_FragColor.a = mat.albedo.a;
}
