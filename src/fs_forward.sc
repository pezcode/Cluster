$input v_texcoord0

#include <bgfx_shader.sh>
#include "pbr.sh"

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texMetallicRoughness, 1);

void main()
{
    gl_FragColor = texture2D(s_texBaseColor, v_texcoord0);
}
