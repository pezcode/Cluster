#define WRITE_LUT

#include "common.sh"
#include <bgfx_compute.sh>
#include "pbr.sh"

// compute shader to calculate a lookup table for multiple scattering correction

// Turquin. 2018. Practical multiple scattering compensation for microfacet models.
// https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf

// some importance-sampling code taken from https://bruop.github.io/ibl/

// Turquin recommends 32x32
// the functions are fairly smooth so this is enough
#define LUT_SIZE 32
#define THREADS LUT_SIZE

// number of samples for approximating the albedo integral
#define NUM_SAMPLES 2048

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
vec2 hammersley_2d(uint i, uint N)
{
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float ri = float(bits) * 2.3283064365386963e-10; // / uintBitsToFloat(0x100000000);
    return vec2(float(i) / float(N), ri);
}

// map two random variables in [0;1] to a point on a hemisphere centered around N = y
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
vec3 sampleHemisphere(vec2 Xi)
{
    // turn Xi into spherical coordinates
    // azimuthal angle (360°)
    float phi = Xi.y * 2.0 * PI;
    // polar angle (90°)
    // 1-u to map the first sample (0) in the Hammersley sequence to 1=cos(0°)=up
    float cosTheta = 1.0 - Xi.x;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    // to cartesian coordinates, y is up
    return vec3(
        cos(phi) * sinTheta,
        cosTheta,
        sin(phi) * sinTheta);
}

// same thing, but importance sampled for the GGX NDF
// return values is the half-way vector H
vec3 importanceSampleGGX(vec2 Xi, float roughness)
{
    float a = roughness * roughness;

    // Sample in spherical coordinates
    float phi = 2.0 * PI * Xi.y;
    float cosTheta = sqrt((1.0 - Xi.x) / (1.0 + (a * a - 1.0) * Xi.x));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // to cartesian coordinates, y is up
    return vec3(
        cos(phi) * sinTheta,
        cosTheta,
        sin(phi) * sinTheta);
}

#define IMPORTANCE_SAMPLE_BRDF 1

// calculate the albedo for a perfectly reflective surface (F0 = 1)
// = the irradiance reflected towards the eye position from uniform
// lighting over the hemisphere
float albedo(float NoV, float roughness)
{
	vec3 V;
    V.x = sqrt(1.0 - NoV * NoV); // sin
    V.y = NoV; // cos
    V.z = 0.0;

    // N points straight upwards (y) for this integration
    const vec3 N = vec3(0.0, 1.0, 0.0);

    float E = 0.0;

    // fixed F0 -> perfectly reflective surface
    // only valid for metals, for dielectrics this needs to be a 3D LUT with F0 as a parameter
    // we could possibly fix F0 at 0.04 like GLTF does, and write a second channel in the LUT
    // how does this work with lerping with the metallic factor?
    const vec3 F0 = vec3_splat(1.0);

    // Monte-Carlo sampling for numerically integrating the albedo over the hemisphere
    // a(V) = integral(brdf(V, L) * dot(N,L))
    for(uint i = 0; i < NUM_SAMPLES; i++)
    {
        // quasirandom values in [0;1]
        // is there a better distribution we can use?
        // see http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
        vec2 Xi = hammersley_2d(i, NUM_SAMPLES);

        #if IMPORTANCE_SAMPLE_BRDF

        // sample microfacet direction
        // this returns H because we sample the NDF, which is the distribution of microfacets, which reflect around H
        // apparently the PDF needs the 4*VoH denominator (related to the Jacobian)
        // clarify!
        vec3 H = importanceSampleGGX(Xi, roughness);

        // get the light direction
        vec3 L = 2.0 * dot(V, H) * H - V;

        #else

        vec3 L = sampleHemisphere(Xi);
        vec3 H = normalize(V + L);

        #endif

        float NoL = saturate(dot(N, L));
        float NoH = saturate(dot(N, H));
        float VoH = saturate(dot(V, H));

        // specular BRDF

        float a = roughness * roughness;
        
        float F = F_Schlick(VoH, F0).x;
        float VF = V_SmithGGXCorrelated(NoV, NoL, a);
        float D = D_GGX(NoH, a);

        #if IMPORTANCE_SAMPLE_BRDF

        //float Fr = F * VF * D;
        //float pdf = D * NoH / (4.0 * VoH);
        //E += Fr * NoL / pdf;
        // -> D cancels out

        E += F * VF * NoL * (4.0 * VoH) / NoH;

        #else

        // this kind of converges at massive sample rates (1 million +)
        // but for low angles and high roughness the output is off
        // investigate!
        float pdf = 0.5 * INV_PI;
        E += F * VF * D * NoL / pdf;

        #endif
    }

    return E / float(NUM_SAMPLES);
}

NUM_THREADS(THREADS, THREADS, 1)
void main()
{
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec2 values = (vec2(coords) + 0.5) / LUT_SIZE;

    float NoV = values.x;
    float roughness = values.y;
    float result = albedo(NoV, roughness);

    result = saturate(result);

    imageStore(i_texAlbedoLUT, coords, vec4(result, result, result, 1.0));
}
