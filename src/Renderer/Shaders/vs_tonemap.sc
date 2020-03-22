$input a_position

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
}
