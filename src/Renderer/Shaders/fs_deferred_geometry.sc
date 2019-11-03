$input v_normal, v_tangent, v_texcoord0

#define READ_MATERIAL

#include <bgfx_shader.sh>
#include "util.sh"
#include "pbr.sh"

void main()
{
    PBRMaterial mat = pbrMaterial(v_texcoord0);
    vec3 N = convertTangentNormal(v_normal, v_tangent, mat.normal);
    mat.a = specularAntiAliasing(N, mat.a);

    // pack G-Buffer
    gl_FragData[0] = vec4(mat.diffuseColor, mat.a);
    // encode normal for unsigned normalized texture
    // w of 1.0 so it gets rendered in the debug visualization
    gl_FragData[1] = vec4(N * 0.5 + 0.5, 1.0);
    gl_FragData[2] = vec4(mat.metallic, mat.F0);
}
