$input a_position

#include <bgfx_shader.sh>

void main()
{
    // view and projection matrix match the geometry pass
    // so that we have the correct inverse matrices for screen -> eye -> world

    // abusing the model matrix to translate the light culling quad
    // from model -> clip
    gl_Position = mul(u_model[0], vec4(a_position, 1.0));
}
