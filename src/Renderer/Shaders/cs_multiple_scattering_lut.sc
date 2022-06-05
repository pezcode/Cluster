#define WRITE_LUT

#include <bgfx_compute.sh>
#include "pbr.sh"

// compute shader to calculate a lookup table for multiple scattering correction

// TODO fix black pixels at grazing angles (NaN?)

// Turquin. 2018. Practical multiple scattering compensation for microfacet models.
// https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf

// some importance-sampling code taken from https://bruop.github.io/ibl/

// Turquin recommends 32x32
// the functions are fairly smooth so this is enough
#define LUT_SIZE 32
#define THREADS LUT_SIZE

// number of samples for approximating the integral
#define NUM_SAMPLES 1024

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
vec2 hammersley_2d(uint i, uint N)
{
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float ri = float(bits) * 2.3283064365386963e-10; // / (1.0 << 32)
    return vec2(float(i) / float(N), ri);
}

// map from [0;1] to a cosine weighted distribution on a hemisphere centered around N = y
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
vec3 sampleHemisphereCosine(vec2 Xi)
{
    // turn Xi into spherical coordinates
    // azimuthal angle (360°)
    float phi = Xi.y * 2.0 * PI;
    // polar angle (90°)
    // 1-u to map the first sample (0) in the Hammersley sequence to 1=cos(0°)=up
    float cosTheta = sqrt(1.0 - Xi.x);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // to cartesian coordinates, y is up
    return vec3(
        cos(phi) * sinTheta,
        cosTheta,
        sin(phi) * sinTheta);
}

// importance sample the hemisphere for the GGX NDF instead
// return value is the half-way vector H
// a is perceptual roughness squared
vec3 sampleGGX(vec2 Xi, float a)
{
    float phi = 2.0 * PI * Xi.y;
    float cosTheta = sqrt((1.0 - Xi.x) / (1.0 + (a * a - 1.0) * Xi.x));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    return vec3(
        cos(phi) * sinTheta,
        cosTheta,
        sin(phi) * sinTheta);
}

#define IMPORTANCE_SAMPLE_BRDF 1

// calculate the directional albedo = the irradiance reflected towards the eye position
// from uniform lighting over the hemisphere
float albedo_specular(vec3 V, float NoV, PBRMaterial mat)
{
    // N points straight upwards (y) for this integration
    const vec3 N = vec3(0.0, 1.0, 0.0);

    float E = 0.0;

    // Monte-Carlo sampling for numerically integrating the directional albedo
    // E(V) = integral(brdf(V,L) * dot(N,L))

    for(uint i = 0; i < NUM_SAMPLES; i++)
    {
        // quasirandom values in [0;1]
        // is there a better distribution we can use?
        // see http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
        vec2 Xi = hammersley_2d(i, NUM_SAMPLES);

        // sample microfacet direction
        // this returns H because we sample the NDF, which is the distribution of microfacets, which reflect around H
        // then the PDF needs the 4*VoH denominator since we evaluate L (related to the Jacobian)
        vec3 H = sampleGGX(Xi, mat.a);

        // get the light direction
        vec3 L = 2.0 * dot(V, H) * H - V;

        float NoL = saturate(dot(N, L));
        float NoH = saturate(dot(N, H));
        float VoH = saturate(dot(V, H));

        // evaluate BRDF

        float F = F_Schlick(VoH, mat.F0).x;
        float VF = V_SmithGGXCorrelated(NoV, NoL, mat.a);

        //float Fr = F * VF * D;
        //float inv_pdf = (4.0 * VoH) / (D * NoH);
        //E += Fr * NoL * inv_pdf;
        // -> D cancels out

        E += F * VF * NoL * 4.0 * VoH / NoH;
    }

    return E / float(NUM_SAMPLES);
}

float albedo_diffuse(vec3 V, float NoV, PBRMaterial mat)
{
    float E = 0.0;

    for(uint i = 0; i < NUM_SAMPLES; i++)
    {
        vec2 Xi = hammersley_2d(i, NUM_SAMPLES);

        vec3 L = sampleHemisphereCosine(Xi);
        vec3 H = normalize(V + L);

        float VoH = saturate(dot(V, H));

        float F = F_Schlick(VoH, mat.F0).x;

        // Fr = (1 - F) * C * (1/pi) * NoL
        // float inv_pdf = pi/NoL
        // -> 1/pi and NoL cancel out

        E += (1.0 - F) * mat.diffuseColor.x;
    }

    return E / float(NUM_SAMPLES);
}

NUM_THREADS(THREADS, THREADS, 1)
void main()
{
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec2 values = (vec2(coords) + 0.5) / LUT_SIZE;

    float NoV = values.x;
    float roughness = sqrt(values.y); // LUT is indexed by a = roughness^2

    vec3 V;
    V.x = sqrt(1.0 - NoV * NoV); // sin
    V.y = NoV; // cos
    V.z = 0.0;

    PBRMaterial mat;
    // D3D compiler insists we initialize everything
    mat.normal = vec3(0.0, 1.0, 0.0);
    mat.occlusion = 0.0;
    mat.emissive = vec3_splat(0.0);
    mat.albedo = vec4_splat(1.0);
    mat.roughness = roughness;

    mat.metallic = 1.0; // F0 = albedo -> perfectly reflective surface
    mat = pbrInitMaterial(mat);
    float albedo_metal = albedo_specular(V, NoV, mat);

    mat.metallic = 0.0; // F0 = 0.04
    mat = pbrInitMaterial(mat);
    float albedo_dielectric = albedo_specular(V, NoV, mat) + albedo_diffuse(V, NoV, mat);

    imageStore(i_texAlbedoLUT, coords, vec4(albedo_metal, albedo_dielectric, 0.0, 1.0));
}
