$input v_eyepos, v_normal, v_tangent, v_bitangent, v_texcoord0

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

uniform vec4 u_sceneScaleVec;
#define u_sceneScale u_sceneScaleVec.x

void main()
{
    const vec3 ambientLightFlux = vec3(0.02, 0.02, 0.02);

    PBRMaterial mat = pbrMaterial(v_texcoord0);

    // normal map

    vec3 normal = v_normal;
    //if(length(mat.normal) != 0.0)
    {
        // convert normal map from tangent space -> eye space (= space of v_tangent, etc.)
        mat3 TBN = mtx3FromCols(
            normalize(v_tangent),
            normalize(v_bitangent),
            normalize(v_normal)
        );
        normal = normalize(mul(TBN, mat.normal));
    }

    // shading

    vec3 camPos = vec3(0.0, 0.0, 0.0);

    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - v_eyepos);

    vec3 radianceOut = vec3_splat(0.0);

    uint lights = pointLightCount();
    for(uint i = 0; i < lights; i++)
    {
        vec3 lightPos = mul(u_view, vec4(pointLightPosition(i), 1.0)).xyz;
        vec3 flux = pointLightFlux(i);

        vec3 L = normalize(lightPos - v_eyepos);
        float dist = distance(lightPos, v_eyepos) / u_sceneScale;
        // TODO basic distance culling
        // needs to factor in flux
        //if(dist < 10.0)
        {
        float attenuation = 1.0 / (dist * dist);
        vec3 radianceIn   = flux * attenuation; 
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        radianceOut += BRDF(V, L, N, mat) * radianceIn * NoL;
        }
    }

    vec3 ambient = ambientLightFlux * mat.albedo.rgb; // * ambientOcclusion
    vec3 color = radianceOut + ambient;
    
    // output goes straight to HDR framebuffer, no clamping
    // tonemapping happens in final blit

    gl_FragColor.rgb = color;
    gl_FragColor.a = mat.albedo.a;
}
