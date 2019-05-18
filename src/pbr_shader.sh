#define SRGB_CONVERSION_FAST
#include "tonemapping.sh"

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texMetallicRoughness, 1);
SAMPLER2D(s_texNormal, 2);

#define PBR_SAMPLER_END 2

uniform vec4 u_baseColorFactor;
uniform vec4 u_metallicRoughnessFactor;
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
        return vec4(sRGBToLinear(color.rgb), color.a) * u_baseColorFactor;
    }
    else
    {
        return u_baseColorFactor;
    }
}

vec2 pbrMetallicRoughness(vec2 texcoord)
{
    if(u_hasMetallicRoughnessTexture)
    {
        return texture2D(s_texMetallicRoughness, texcoord).bg * u_metallicRoughnessFactor.xy;
    }
    else
    {
        return u_metallicRoughnessFactor.xy;
    }
}

vec3 pbrNormal(vec2 texcoord)
{
    if(u_hasNormalTexture)
    {
        return normalize((texture2D(s_texNormal, texcoord).rgb * 2.0) - 1.0);
    }
    else
    {
        // should this be something else to match null vector in tangent space?
        return vec3_splat(0.0);
    }
}

struct PBRMaterial
{
    vec4 albedo;
    float metallic;
    float roughness;
    vec3 normal;
};

PBRMaterial pbrMaterial(vec2 texcoord)
{
    PBRMaterial mat;
    mat.albedo = pbrBaseColor(texcoord);
    vec2 metallicRoughness = pbrMetallicRoughness(texcoord);
    mat.metallic  = metallicRoughness.r;
    mat.roughness = metallicRoughness.g;
    mat.normal = pbrNormal(texcoord);
    return mat;
}

// Physically based shading
// Metallic + roughness workflow (GLTF 2.0 core material spec)
// BRDF, no sub-surface scattering
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#metallic-roughness-material
// Some GLSL code taken from
// https://google.github.io/filament/Filament.md.html
// and
// https://learnopengl.com/PBR/Lighting

#define PI 3.1415926535897932384626433832795

// Schlick approximation to Fresnel equation
vec3 F_Schlick(float VoH, vec3 F0)
{
    float f = pow(1.0 - VoH, 5.0);
    return f + F0 * (1.0 - f);
}

// Normal Distribution Function
// (aka specular distribution)
// distribution of microfacets

// Bruce Walter et al. 2007. Microfacet Models for Refraction through Rough Surfaces.
// equivalent to Trowbridge-Reitz
float D_GGX(float NoH, float a)
{
    a = NoH * a;
    float k = a / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

// Geometric Shadowing Function

// GGX version (see above)
// based on Smith approach
float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

// Lambertian diffuse BRDF
// uniform color
float Fd_Lambert()
{
    return 1.0 / PI;
}

vec3 BRDF(vec3 v, vec3 l, vec3 n, PBRMaterial mat)
{
    // GLTF 2.0

    const vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
    const vec3 black = vec3(0.0, 0.0, 0.0);

    vec3 diffuseColor = mix(mat.albedo.rgb * (1.0 - dielectricSpecular.r), black, mat.metallic);
    vec3 F0 = mix(dielectricSpecular, mat.albedo.rgb, mat.metallic);
    float a = mat.roughness * mat.roughness;

    /*
    V is the normalized vector from the shading location to the eye
    L is the normalized vector from the shading location to the light
    N is the surface normal in the same space as the above values
    H is the half vector, where H = normalize(L+V)
    */

    vec3 h = normalize(l + v);

    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    // specular BRDF
    float D = D_GGX(NoH, a);
    vec3  F = F_Schlick(LoH, F0);
    float V = V_SmithGGXCorrelated(NoV, NoL, a);
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    vec3 Fd = diffuseColor * Fd_Lambert();

    vec3 kD = (1.0 - F) * (1.0 - mat.metallic);
    return kD * Fd + Fr;
}
