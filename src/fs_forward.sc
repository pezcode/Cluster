$input v_normal, v_tangent, v_texcoord0

#include <bgfx_shader.sh>
#include "pbr.sh"

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texMetallicRoughness, 1);
SAMPLER2D(s_texNormal, 2);

uniform vec4 u_baseColor;
uniform vec4 u_metallicRoughness;
uniform vec4 u_hasTextures;

#define u_metallic u_metallicRoughness.x
#define u_roughness u_metallicRoughness.y

#define u_hasBaseColorTexture (u_hasTextures.x != 0.0)
#define u_hasMetallicRoughnessTexture (u_hasTextures.y != 0.0)
#define u_hasNormalTexture (u_hasTextures.z != 0.0)

void main()
{
    vec4 baseColor = u_hasBaseColorTexture ? texture2D(s_texBaseColor, v_texcoord0) : u_baseColor;
    vec2 metallicRoughness = u_hasMetallicRoughnessTexture ? texture2D(s_texMetallicRoughness, v_texcoord0).xy : u_metallicRoughness.xy;
    float metallic = u_metallic;
    float roughness = u_roughness;
    vec3 normal = u_hasNormalTexture ? texture2D(s_texNormal, v_texcoord0).xyz : vec3(0.0, 0.0, 0.0);
    gl_FragColor = baseColor;
}
