$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0
$output v_eyepos,  v_normal, v_tangent, v_bitangent, v_texcoord0

#include <bgfx_shader.sh>

// modelView transformation for normals to preserve perpendicularity
uniform mat3 u_normalMatrix;

void main()
{
    v_eyepos = mul(u_modelView, vec4(a_position, 1.0)).xyz;
    v_normal = mul(u_normalMatrix, a_normal);
    v_tangent = mul(u_modelView, vec4(a_tangent, 0.0)).xyz;
    v_bitangent = mul(u_modelView, vec4(a_bitangent, 0.0)).xyz;
    v_texcoord0 = a_texcoord0;
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
