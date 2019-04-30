$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main()
{
    gl_FragColor = vec4(v_texcoord0, 0.0, 0.5);
    //gl_FragColor = texture2D(s_texColor, v_texcoord0);
}
