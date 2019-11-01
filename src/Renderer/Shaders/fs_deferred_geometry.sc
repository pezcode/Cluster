$input v_worldpos, v_normal, v_tangent, v_texcoord0

#include <bgfx_shader.sh>
#include "util.sh"
#include "pbr.sh"

void main()
{
    PBRMaterial mat = pbrMaterial(v_texcoord0);

    vec3 bitangent = cross(v_normal, v_tangent);
    mat3 TBN = mtxFromCols(
        normalize(v_tangent),
        normalize(bitangent),
        normalize(v_normal)
    );
    vec3 N = normalize(mul(TBN, mat.normal));

    mat.a = specularAntiAliasing(N, mat.a);

    // pack G-Buffer

    gl_FragData[0] = vec4(mat.diffuseColor, mat.a);
    gl_FragData[1] = vec4(N, 1.0);
    gl_FragData[2] = vec4(mat.metallic, mat.F0);
}
