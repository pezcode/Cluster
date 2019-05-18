$input v_eyepos, v_normal, v_tangent, v_bitangent, v_texcoord0

// all unit-vectors need to be normalized in the fragment shader, the interpolation of vertex shader output doesn't preserve length

#include <bgfx_shader.sh>
#include "pbr_shader.sh"

uniform vec4 u_sceneScaleVec;
#define u_sceneScale u_sceneScaleVec.x

void main()
{
    // why can't these be global?
    vec3 lightPos = vec3(0.10, 0.10, 0.10);
    lightPos = mul(u_view, vec4(lightPos, 1.0)).xyz;
    const vec3 lightFlux = vec3(20.0, 20.0, 20.0); // RGB
    const vec3 ambientLightFlux = vec3(0.01, 0.01, 0.01);

    PBRMaterial mat = pbrMaterial(v_texcoord0);

    // do shading

    vec3 camPos = vec3(0.0, 0.0, 0.0);

    // convert normal map from tangent space -> eye space (= space of v_tangent, etc.)
    mat3 TBN = mat3(
        normalize(v_tangent),
        normalize(v_bitangent),
        normalize(v_normal)
    ); // use mtxFromCols? mat4 :(
#if !BGFX_SHADER_LANGUAGE_GLSL // GLSL mat3 constructor takes columns
    TBN = transpose(TBN);
#endif
    vec3 normalOffset = normalize(mul(TBN, mat.normal));

    vec3 N = normalize(v_normal + normalOffset);
    vec3 V = normalize(camPos - v_eyepos);

    vec3 radianceOut = vec3_splat(0.0);
    // for each light:
    {
        vec3 L = normalize(lightPos - v_eyepos);
        float distance    = length(lightPos - v_eyepos) / u_sceneScale;
        float attenuation = 1.0 / (distance * distance);
        vec3 radianceIn   = lightFlux * attenuation; 
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        radianceOut += BRDF(V, L, N, mat) * radianceIn * NoL;
    }

    vec3 ambient = ambientLightFlux * mat.albedo.rgb; // * ambientOcclusion
    vec3 color = radianceOut + ambient;
    
    // output goes straight to HDR framebuffer, no clamping
    // tonemapping happens in final blit

    gl_FragColor.rgb = color;
    gl_FragColor.a = mat.albedo.a;
}
