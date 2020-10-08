$input v_worldpos, v_normal, v_tangent, v_texcoord0

#define READ_MATERIAL

#include "common.sh"
#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "util.sh"
#include "pbr.sh"
#include "lights.sh"
#include "clusters.sh"
#include "colormap.sh"

uniform vec4 u_camPos;

void main()
{
    // the clustered shading fragment shader is almost identical to forward shading
    // first we determine the cluster id from the fragment's window coordinates
    // light count is read from the grid instead of a uniform
    // light indices are read and looped over starting from the grid offset

    PBRMaterial mat = pbrMaterial(v_texcoord0);
    vec3 N = convertTangentNormal(v_normal, v_tangent, mat.normal);
    mat.a = specularAntiAliasing(N, mat.a);

    vec3 camPos = u_camPos.xyz;
    vec3 fragPos = v_worldpos;

    vec3 V = normalize(camPos - fragPos);
    float NoV = abs(dot(N, V)) + 1e-5;
    vec3 msFactor = multipleScatteringFactor(mat, NoV);

    vec3 radianceOut = vec3_splat(0.0);

    uint cluster = getClusterIndex(gl_FragCoord);
    LightGrid grid = getLightGrid(cluster);
    for(uint i = 0; i < grid.pointLights; i++)
    {
        uint lightIndex = getGridLightIndex(grid.offset, i);
        PointLight light = getPointLight(lightIndex);
        float dist = distance(light.position, fragPos);
        float attenuation = smoothAttenuation(dist, light.radius);
        if(attenuation > 0.0)
        {
            vec3 L = normalize(light.position - fragPos);
            vec3 radianceIn = light.intensity * attenuation;
            float NoL = saturate(dot(N, L));
            radianceOut += BRDF(V, L, N, NoV, NoL, mat) * msFactor * radianceIn * NoL;
        }
    }

    radianceOut += getAmbientLight().irradiance * mat.diffuseColor * mat.occlusion;
    radianceOut += mat.emissive;

    gl_FragColor.rgb = radianceOut;
    gl_FragColor.a = mat.albedo.a;
}
