#ifndef PBR_SH_HEADER_GUARD
#define PBR_SH_HEADER_GUARD

#include "samplers.sh"

// only define this if you need to retrieve the material parameters
// without it you can still use the struct definition or BRDF functions
#ifdef READ_MATERIAL

SAMPLER2D(s_texBaseColor,         SAMPLER_PBR_BASECOLOR);
SAMPLER2D(s_texMetallicRoughness, SAMPLER_PBR_METALROUGHNESS);
SAMPLER2D(s_texNormal,            SAMPLER_PBR_NORMAL);
SAMPLER2D(s_texOcclusion,         SAMPLER_PBR_OCCLUSION);
SAMPLER2D(s_texEmissive,          SAMPLER_PBR_EMISSIVE);

uniform vec4 u_baseColorFactor;
uniform vec4 u_metallicRoughnessNormalOcclusionFactor;
uniform vec4 u_emissiveFactorVec;
uniform vec4 u_hasTextures1;
uniform vec4 u_hasTextures2;

#define u_hasBaseColorTexture         (u_hasTextures1.x != 0.0f)
#define u_hasMetallicRoughnessTexture (u_hasTextures1.y != 0.0f)
#define u_hasNormalTexture            (u_hasTextures1.z != 0.0f)
#define u_hasOcclusionTexture         (u_hasTextures1.w != 0.0f)
#define u_hasEmissiveTexture          (u_hasTextures2.x != 0.0f)

#define u_metallicRoughnessFactor (u_metallicRoughnessNormalOcclusionFactor.xy)
#define u_normalScale             (u_metallicRoughnessNormalOcclusionFactor.z)
#define u_occlusionStrength       (u_metallicRoughnessNormalOcclusionFactor.w)
#define u_emissiveFactor          (u_emissiveFactorVec.xyz)

#endif

struct PBRMaterial
{
    vec4 albedo;
    float metallic;
    float roughness;
    vec3 normal;
    float occlusion;
    vec3 emissive;

    // calculated from the above

    vec3 diffuseColor; // this becomes black for higher metalness
    vec3 F0; // Fresnel reflectance at normal incidence
    float a; // remapped roughness (^2)
};

#ifdef READ_MATERIAL

vec4 pbrBaseColor(vec2 texcoord)
{
    if(u_hasBaseColorTexture)
    {
        return texture2D(s_texBaseColor, texcoord) * u_baseColorFactor;
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
        return texture2D(s_texMetallicRoughness, texcoord).bg * u_metallicRoughnessFactor;
    }
    else
    {
        return u_metallicRoughnessFactor;
    }
}

vec3 pbrNormal(vec2 texcoord)
{
    if(u_hasNormalTexture)
    {
        // the normal scale can cause problems and serves no real purpose
        // normal compression and BRDF calculations assume unit length
        return normalize((texture2D(s_texNormal, texcoord).rgb * 2.0) - 1.0); // * u_normalScale;
    }
    else
    {
        // the up vector (w) in tangent space is the default normal
        // that's why normal maps are mostly blue
        return vec3(0.0, 0.0, 1.0);
    }
}

float pbrOcclusion(vec2 texcoord)
{
    if(u_hasOcclusionTexture)
    {
        // occludedColor = lerp(color, color * <sampled occlusion texture value>, <occlusion strength>)
        float occlusion = texture2D(s_texOcclusion, texcoord).r;
        return occlusion + (1.0 - occlusion) * (1.0 - u_occlusionStrength);
    }
    else
    {
        return 1.0;
    }
}

vec3 pbrEmissive(vec2 texcoord)
{
    if(u_hasEmissiveTexture)
    {
        return texture2D(s_texEmissive, texcoord).rgb * u_emissiveFactor;
    }
    else
    {
        return u_emissiveFactor;
    }
}

PBRMaterial pbrInitMaterial(PBRMaterial mat);

PBRMaterial pbrMaterial(vec2 texcoord)
{
    PBRMaterial mat;

    // Read textures/uniforms

    mat.albedo = pbrBaseColor(texcoord);
    vec2 metallicRoughness = pbrMetallicRoughness(texcoord);
    mat.metallic  = metallicRoughness.r;
    mat.roughness = metallicRoughness.g;
    mat.normal = pbrNormal(texcoord);
    mat.occlusion = pbrOcclusion(texcoord);
    mat.emissive = pbrEmissive(texcoord);

    mat = pbrInitMaterial(mat);

    return mat;
}

#endif // READ_MATERIAL

PBRMaterial pbrInitMaterial(PBRMaterial mat)
{
    // Taken directly from GLTF 2.0 specs
    // this can be precalculated instead of evaluating it in the BRDF for every light

    const vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
    const vec3 black = vec3(0.0, 0.0, 0.0);

    mat.diffuseColor = mix(mat.albedo.rgb * (vec3_splat(1.0) - dielectricSpecular), black, mat.metallic);
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

#define INV_PI (0.3183098861837906715377675267450)

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
// height-correlated joint masking-shadowing function:
// Heitz 2014. Understanding the Masking-Shadowing Functionin Microfacet-Based BRDFs.
// http://jcgt.org/published/0003/02/03/paper.pdf
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
