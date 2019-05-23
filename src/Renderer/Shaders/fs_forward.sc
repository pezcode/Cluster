$input v_worldpos, v_normal, v_tangent, v_bitangent, v_texcoord0

// all unit-vectors need to be normalized in the fragment shader, the interpolation of vertex shader output doesn't preserve length

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "pbr.sh"
#include "lights.sh"

mat3 mtx3FromCols(vec3 c0, vec3 c1, vec3 c2)
{
// GLSL mat3 constructor takes columns
#if BGFX_SHADER_LANGUAGE_GLSL
	return mat3(c0, c1, c2);
#else
	return transpose(mat3(c0, c1, c2));
#endif
}

uniform vec4 u_camPos;

void main()
{
    const vec3 ambientLightpower = vec3(0.02, 0.02, 0.02);

    PBRMaterial mat = pbrMaterial(v_texcoord0);

    // normal map

    // convert normal map from tangent space -> world space (= space of v_tangent, etc.)
    mat3 TBN = mtx3FromCols(
        normalize(v_tangent),
        normalize(v_bitangent),
        normalize(v_normal)
    );
    vec3 N = normalize(mul(TBN, mat.normal));
    
    // shading

    // TODO determine light radius automatically
    // or send as z component
    const float maxLightRadius = 10.0;

    vec3 camPos = u_camPos;
    vec3 fragPos = v_worldpos;

    vec3 V = normalize(camPos - fragPos);

    vec3 radianceOut = vec3_splat(0.0);

    uint lights = pointLightCount();
    for(uint i = 0; i < lights; i++)
    {
        vec3 lightPos = pointLightPosition(i);
        float dist = distance(lightPos, fragPos);
        float attenuation = smoothAttenuation(dist, maxLightRadius);
        if(attenuation > 0.0)
        {
            vec3 intensity = pointLightIntensity(i);
            vec3 L = normalize(lightPos - fragPos);
            vec3 radianceIn = intensity * attenuation;
            float NoL = clamp(dot(N, L), 0.0, 1.0);
            radianceOut += BRDF(V, L, N, mat) * radianceIn * NoL;
        }
    }

    vec3 ambient = ambientLightpower * mat.albedo.rgb; // * ambientOcclusion
    vec3 color = radianceOut + ambient;
    
    // output goes straight to HDR framebuffer, no clamping
    // tonemapping happens in final blit

    gl_FragColor.rgb = color;
    gl_FragColor.a = mat.albedo.a;
}
