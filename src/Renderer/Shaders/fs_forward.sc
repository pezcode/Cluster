$input v_worldpos, v_normal, v_tangent, v_texcoord0

// all unit-vectors need to be normalized in the fragment shader, the interpolation of vertex shader output doesn't preserve length

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "util.sh"
#include "pbr.sh"
#include "lights.sh"

uniform vec4 u_camPos;

void main()
{
    PBRMaterial mat = pbrMaterial(v_texcoord0);

    // normal map

    // convert normal map from tangent space -> world space (= space of v_tangent, etc.)
    vec3 bitangent = cross(v_normal, v_tangent);
    mat3 TBN = mtx3FromCols(
        normalize(v_tangent),
        normalize(bitangent),
        normalize(v_normal)
    );
    vec3 N = normalize(mul(TBN, mat.normal));

    mat.a = specularAntiAliasing(N, mat.a);

    // shading

    vec3 camPos = u_camPos.xyz;
    vec3 fragPos = v_worldpos;

    vec3 V = normalize(camPos - fragPos);

    vec3 radianceOut = vec3_splat(0.0);

    uint lights = pointLightCount();
    for(uint i = 0; i < lights; i++)
    {
        PointLight light = getPointLight(i);
        float dist = distance(light.position, fragPos);
        if(dist < light.radius)
        {
            float attenuation = smoothAttenuation(dist, light.radius);
            vec3 L = normalize(light.position - fragPos);
            vec3 radianceIn = light.intensity * attenuation;
            float NoL = clamp(dot(N, L), 0.0, 1.0);
            radianceOut += BRDF(V, L, N, mat) * radianceIn * NoL;
        }
    }

    vec3 ambient = getAmbientLight().irradiance * mat.albedo.rgb; // * ambientOcclusion
    vec3 color = radianceOut + ambient;

    // output goes straight to HDR framebuffer, no clamping
    // tonemapping happens in final blit

    gl_FragColor.rgb = color;
    gl_FragColor.a = mat.albedo.a;
}
