$input a_position

#include <bgfx_shader.sh>

void main()
{
    // fullscreen triangle is already transformed to clip space, nothing to do
    // set z to the far plane (= 1.0 in clip space)
    gl_Position = vec4(a_position.xy, 1.0, 1.0);
}
