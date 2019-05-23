$input v_worldpos, v_normal, v_tangent, v_bitangent, v_texcoord0

// all unit-vectors need to be normalized in the fragment shader, the interpolation of vertex shader output doesn't preserve length

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "pbr.sh"
#include "lights.sh"
#include "clusters.sh"

uniform vec4 u_clusterSliceScaleBiasVec;
#define u_clusterSliceScale u_clusterSliceScaleBiasVec.x
#define u_clusterSliceBias  u_clusterSliceScaleBiasVec.y

void main()
{
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
    // TODO send uniforms
    float scale = DEPTH_SLICES / log(5.0 / 0.1); //u_clusterSliceScale;
    float bias = -DEPTH_SLICES * log(0.1) / log(5.0 / 0.1); //u_clusterSliceBias;

    vec3 eyepos = mul(u_view, vec4(v_worldpos, 1.0)).xyz;

    uint slice = uint(log(eyepos.z) * scale + u_clusterSliceBias);
    vec3 color = colors[slice % 6];

    gl_FragColor = vec4(color, 1.0);
}
