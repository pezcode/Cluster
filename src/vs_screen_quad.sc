$input a_position
$output v_texcoord0

#include <bgfx_shader.sh>

vec2 clip_to_uv(vec2 val)
{
    return (val + 1.0f) * 0.5f;
}

void main()
{
    gl_Position = vec4(a_position, 1.0);
    v_texcoord0 = clip_to_uv(a_position.xy);
}
