$input v_normal, v_tangent, v_texcoord0

#include <bgfx_shader.sh>

#define SRGB_CONVERSION_FAST
#include "tonemapping.sh"
#include "pbr_shader.sh"

void main()
{
    vec4 baseColor = pbrBaseColor(v_texcoord0);
    vec2 metallicRoughness = pbrMetallicRoughness(v_texcoord0);
    float metallic = metallicRoughness.x;
    float roughness = metallicRoughness.y;
    vec3 normal = pbrNormal(v_texcoord0);

    // do shading
    vec4 result = baseColor;

    // output goes to HDR framebuffer, so don't clamp
    // tonemapping happens in final blit

    gl_FragColor = result;
}
