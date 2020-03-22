$input a_position

#include "common.sh"
#include <bgfx_shader.sh>

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    // prevent far plane clipping for point light volumes outside the camera frustum
    //gl_Position.z = min(gl_Position.z / gl_Position.w, 1.0) * gl_Position.w;
}
