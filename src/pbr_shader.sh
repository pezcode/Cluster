SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texMetallicRoughness, 1);
SAMPLER2D(s_texNormal, 2);

uniform vec4 u_baseColor;
uniform vec4 u_metallicRoughness;
uniform vec4 u_hasTextures;

#define u_hasBaseColorTexture         (u_hasTextures.x != 0.0f)
#define u_hasMetallicRoughnessTexture (u_hasTextures.y != 0.0f)
#define u_hasNormalTexture            (u_hasTextures.z != 0.0f)

vec4 pbrBaseColor(vec2 texcoord)
{
    if(u_hasBaseColorTexture)
    {
        vec4 color = texture2D(s_texBaseColor, texcoord);
		// GLTF base color texture is stored as sRGB
        return vec4(sRGBToLinear(color.rgb), color.a);
    }
	else
	{
		return u_baseColor;
	}
}

vec3 pbrNormal(vec2 texcoord)
{
	if(u_hasNormalTexture)
	{
		return texture2D(s_texNormal, texcoord).xyz;
	}
	else
	{
		// should this be something else to match null vector in tangent space?
		return vec3_splat(0.0);
	}
}

vec2 pbrMetallicRoughness(vec2 texcoord)
{
	if(u_hasMetallicRoughnessTexture)
	{
		return texture2D(s_texMetallicRoughness, texcoord).xy;
	}
	else
	{
		return u_metallicRoughness.xy;
	}
}
