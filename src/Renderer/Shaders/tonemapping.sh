#ifndef TONEMAPPING_SH_HEADER_GUARD
#define TONEMAPPING_SH_HEADER_GUARD

// relative luminance of linear RGB(!)
// BT.709 primaries
float luminance(vec3 RGB)
{
    return 0.2126 * RGB.r + 0.7152 * RGB.g + 0.0722 * RGB.b;
}

// Reinhard, Hable, Duiken taken from:
// http://filmicworlds.com/blog/filmic-tonemapping-operators/

// Exponential tone mapping
vec3 tonemap_exponential(vec3 color)
{
    return vec3_splat(1.0) - exp(-color);
}

// Reinhard et al
// http://www.cs.utah.edu/~reinhard/cdrom/tonemap.pdf
// simple version, desaturates colors

vec3 tonemap_reinhard(vec3 color)
{
    return color / (color + 1.0);
}

// Reinhard, luminance only
// possibly creates undesirable whites
// one alternative is to define a pure white point (ideally max luminance in the scene)
// see original paper and https://imdoingitwrong.wordpress.com/2010/08/19/why-reinhard-desaturates-my-blacks-3/

vec3 tonemap_reinhard_luminance(vec3 color)
{
    float lum = luminance(color);
    float nLum =  lum / (lum + 1.0);
    return color * (nLum / lum);
}

// Uncharted 2 filmic operator
// John Hable
// https://www.gdcvault.com/play/1012351/Uncharted-2-HDR
// https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting

vec3 hable_map(vec3 x)
{
	// values used are directly from the presentation
	// comments have the values taken from the website above
	const float A = 0.22; // shoulder strength // 0.15
	const float B = 0.30; // linear strength // 0.50
	const float C = 0.10; // linear angle
	const float D = 0.20; // toe strength
	const float E = 0.01; // toe numerator // 0.02
	const float F = 0.30; // toe denominator
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 tonemap_hable(vec3 color)
{
	//const float W = 11.2; // linear white point
	//vec3 whiteScale = hable_map(vec3_splat(W));
	const float whiteScale = 0.72513;
	const float ExposureBias = 2.0;
	return hable_map(ExposureBias * color) / whiteScale;
}

// Mimics response curve of Kodak film
// Haarm-Pieter Duiker
// approximation by Hejl/Burgess-Dawson (pow 1/2.2 baked in)
vec3 tonemap_duiker(vec3 color)
{
    vec3 x = max(color - 0.004, 0.0);
    vec3 result = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
    return pow(result, vec3_splat(2.2));
}

// Polynomial fit of ACES
// Stephen Hill
// Taken from:
// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
vec3 tonemap_aces(vec3 color)
{
    // sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
    // sRGB refers to gamut, not display transform
    const mat3 ACESInputMat = mtxFromCols3(
        vec3(0.59719, 0.07600, 0.02840),
        vec3(0.35458, 0.90834, 0.13383),
        vec3(0.04823, 0.01566, 0.83777)
    );

    // ODT_SAT => XYZ => D60_2_D65 => sRGB
    const mat3 ACESOutputMat = mtxFromCols3(
        vec3(1.60475, -0.10208, -0.00327),
        vec3(-0.53108, 1.10813, -0.07276),
        vec3(-0.07367, -0.00605, 1.07602)
    );

    // colors in this code are premultiplied by 1.8
    // https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ToneMapping.hlsl
    // color *= 1.8;
    vec3 result = mul(ACESInputMat, color);

    // RRT and ODT
    vec3 v = result;
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    color = a / b;

    result = mul(ACESOutputMat, color);
    return saturate(result);
}

// Luminance only fit of ACES
// Oversatures brights similar to Reinhard in luminance
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 tonemap_aces_luminance(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    vec3 x = color * 0.6;
    return saturate((x * (a * x + b)) / (x * (c * x + d ) + e));
}

#endif // TONEMAPPING_SH_HEADER_GUARD
