#ifndef PBR_SH_HEADER_GUARD
#define PBR_SH_HEADER_GUARD

#include "samplers.sh"
#include "tonemapping.sh"

SAMPLER2D(s_texBaseColor,         SAMPLER_PBR_BASECOLOR);
SAMPLER2D(s_texMetallicRoughness, SAMPLER_PBR_METALROUGHNESS);
SAMPLER2D(s_texNormal,            SAMPLER_PBR_NORMAL);

uniform vec4 u_baseColorFactor;
uniform vec4 u_metallicRoughnessFactor;
uniform vec4 u_hasTextures;

#define u_hasBaseColorTexture         (u_hasTextures.x != 0.0f)
#define u_hasMetallicRoughnessTexture (u_hasTextures.y != 0.0f)
#define u_hasNormalTexture            (u_hasTextures.z != 0.0f)

struct PBRMaterial
{
    vec4 albedo;
    float metallic;
    float roughness;
    vec3 normal;

    // GLTF 2.0 inputs

    vec3 diffuseColor; // this becomes black for higher metalness
    vec3 F0; // Fresnel reflectance at normal incidence
    float a; // remapped roughness (^2)
};

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
        // the up vector (w) in tangent space is the default normal
        // that's why normal maps are mostly blue
        return vec3(0.0, 0.0, 1.0);
    }
}

PBRMaterial pbrMaterial(vec2 texcoord)
{
    PBRMaterial mat;

    // Read textures/uniforms

    mat.albedo = pbrBaseColor(texcoord);
    vec2 metallicRoughness = pbrMetallicRoughness(texcoord);
    mat.metallic  = metallicRoughness.r;
    mat.roughness = metallicRoughness.g;
    mat.normal = pbrNormal(texcoord);

    // Taken directly from GLTF 2.0 specs
    // this can be precalculated instead of evaluating it in the BRDF for every light

    const vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
    const vec3 black = vec3(0.0, 0.0, 0.0);

    mat.diffuseColor = mix(mat.albedo.rgb * (1.0 - dielectricSpecular.r), black, mat.metallic);
    mat.F0 = mix(dielectricSpecular, mat.albedo.rgb, mat.metallic);
    mat.a = mat.roughness * mat.roughness;

    return mat;
}

// gives a new value a (roughness^2)
float specularAntiAliasing(vec3 N, float a)
{
    // http://www.jp.square-enix.com/tech/library/pdf/ImprovedGeometricSpecularAA.pdf
    // normal-based isotropic filtering
    // this is originally meant for deferred rendering but is a bit simpler to implement than the forward version
    // saves us from calculating uv offsets and sampling textures for every light

    const float SIGMA2 = 0.25; // squared std dev of pixel filter kernel (in pixels)
    const float KAPPA  = 0.18; // clamping threshold

    vec3 dndu = dFdx(N);
    vec3 dndv = dFdy(N);
    float variance = SIGMA2 * (dot(dndu, dndu) + dot(dndv, dndv));
    float kernelRoughness2 = min(2.0 * variance, KAPPA);
    float filteredRoughness2 = saturate(a + kernelRoughness2);
    a = filteredRoughness2;

    // Frostbite clamps roughness to 0.045 (0.045^2 = 0.002025)
    return max(a, 0.002025);
}

// Physically based shading
// Metallic + roughness workflow (GLTF 2.0 core material spec)
// BRDF, no sub-surface scattering
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#metallic-roughness-material
// Some GLSL code taken from
// https://google.github.io/filament/Filament.md.html
// and
// https://learnopengl.com/PBR/Lighting

#define INV_PI 0.3183098861837906715377675267450

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
    return k * k * INV_PI;
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
    // normalize to conserve energy
    // cos integrates to pi over the hemisphere
    // incoming light is multiplied by cos and BRDF
    return INV_PI;
}

vec3 BRDF(vec3 v, vec3 l, vec3 n, PBRMaterial mat)
{
    // V is the normalized vector from the shading location to the eye
    // L is the normalized vector from the shading location to the light
    // N is the surface normal in the same space as the above values
    // H is the half vector, where H = normalize(L+V)

    vec3 h = normalize(l + v);

    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = saturate(dot(n, l));
    float NoH = saturate(dot(n, h));
    float LoH = saturate(dot(l, h));

    // specular BRDF
    float D = D_GGX(NoH, mat.a);
    vec3 F = F_Schlick(LoH, mat.F0);
    float V = V_SmithGGXCorrelated(NoV, NoL, mat.a);
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    vec3 Fd = mat.diffuseColor * Fd_Lambert();

    vec3 kD = (1.0 - F) * (1.0 - mat.metallic);
    return kD * Fd + Fr;
}

#endif // PBR_SH_HEADER_GUARD
