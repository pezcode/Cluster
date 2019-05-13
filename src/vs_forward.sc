$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0
$output v_worldpos, v_normal, v_tangent, v_bitangent, v_texcoord0

#include <bgfx_shader.sh>

void main()
{
    v_worldpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    // this works because the model matrix has uniform scale
    // if that's not the case we need the correct normal matrix
    // (transpose of inverse of model matrix)
    v_normal = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
    v_tangent = mul(u_model[0], vec4(a_tangent, 0.0)).xyz;
    v_texcoord0 = a_texcoord0;
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
