$input v_worldpos, v_eyepos, v_normal, v_tangent, v_texcoord0

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "util.sh"
#include "pbr.sh"
#include "lights.sh"
#include "clusters.sh"

float linearDepth(float depthSample);
uint getClusterIndex(vec4 fragCoord);

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

    vec4 fakeCoord = vec4(gl_FragCoord.x, gl_FragCoord.y, (v_eyepos.z), 1.0);
    uint cluster = getClusterIndex(fakeCoord);
    LightGrid grid = getLightGrid(cluster);
    for(uint i = 0; i < grid.pointLights; i++)
    {
        uint lightIndex = getGridLightIndex(grid.offset, i);
        PointLight light = getPointLight(lightIndex);
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

    vec3 ambient = getAmbientLight().irradiance * mat.albedo.rgb;
    vec3 color = radianceOut + ambient;

    gl_FragColor.rgb = color;
    gl_FragColor.a = mat.albedo.a;

    // debug visualization

    vec3 colors[] = {
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0),
        vec3(1.0, 1.0, 0.0),
        vec3(0.0, 1.0, 1.0),
        vec3(1.0, 0.0, 1.0)
    };

    // http://www.aortiz.me/2018/12/21/CG.html

    // http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf
    float scale = CLUSTERS_Z / log(u_zFar / u_zNear); //u_clusterSliceScale;
    float bias = -CLUSTERS_Z * log(u_zNear) / log(u_zFar / u_zNear); //u_clusterSliceBias;

    uint slice = uint(log(v_eyepos.z) * scale + bias);

    fakeCoord = vec4(gl_FragCoord.x, gl_FragCoord.y, (v_eyepos.z), 1.0);
    slice = getClusterIndex(fakeCoord);

    vec3 debugcolor = colors[slice % 6];

    //gl_FragColor = vec4(debugcolor, 1.0);
}

float linearDepth(float depthSample)
{
    #if BGFX_SHADER_LANGUAGE_GLSL
    float depthRange = 2.0 * depthSample - 1.0; // to NDC -> [-1, 1]
    #else
    float depthRange = depthSample;
    #endif
    // Near... Far... wherever you are...
    // TODO
    vec4 unprojected = mul(u_invProj, vec4(0, 0, depthRange, 1.0));
    return unprojected.z / unprojected.w;
    //float xlinear = 2.0 * u_zNear * u_zFar / (u_zFar + u_zNear - depthRange * (u_zFar - u_zNear));
    //return xlinear;
}

float map(float value, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

uint getClusterIndex(vec4 fragCoord)
{
    float scale = float(CLUSTERS_Z) / log(u_zFar / u_zNear);
    float bias    = -(float(CLUSTERS_Z) * log(u_zNear) / log2(u_zFar / u_zNear));
    /*
    //Locating which cluster you are a part of
    uint zTile     = uint(max(log2(linearDepth(fragCoord.z)) * scale + bias, 0.0));
    */

    uint zTile = uint(max(log2(fragCoord.z) * scale + bias, 0.0));

    //uint zTile = uint(map(log2(fragCoord.z), u_zNear, u_zFar, 0.0, float(CLUSTERS_Z - 1)));
    //uint zTile = uint(fragCoord.z * (float)CLUSTERS_Z / (u_zFar - u_zNear)); // actually eyepos

    //uint zTile = uint(log2(fragCoord.z) * scale + bias);

    uvec3 tiles    = uvec3(uvec2(fragCoord.xy / u_clusterSizes[3]), zTile);
    uint tileIndex = tiles.x +
                     CLUSTERS_X * tiles.y +
                     (CLUSTERS_X * CLUSTERS_Y) * tiles.z;

    return tileIndex;
}
