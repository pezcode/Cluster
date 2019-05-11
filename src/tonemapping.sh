// Taken from
// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ToneMapping.hlsl

vec3 LinearTosRGB(vec3 color)
{
#ifdef SRGB_CONVERSION_FAST
	// approximate sRGB with a gamma correction
	return pow(color, vec3_splat(1.0/2.2));
#else
	vec3 clr = color;
    vec3 x = color * 12.92f;
    vec3 y = 1.055f * pow(saturate(color), 1.0f / 2.4f) - 0.055f;
    clr.r = color.r < 0.0031308f ? x.r : y.r;
    clr.g = color.g < 0.0031308f ? x.g : y.g;
    clr.b = color.b < 0.0031308f ? x.b : y.b;
	return clr;
#endif
}

vec3 sRGBToLinear(vec3 color)
{
#ifdef SRGB_CONVERSION_FAST
	return pow(color, vec3_splat(2.2));
#else
	vec3 clr = color;
    vec3 x = color / 12.92f;
    vec3 y = pow(max((color + 0.055f) / 1.055f, 0.0f), 2.4f);
    clr.r = color.r <= 0.04045f ? x.r : y.r;
    clr.g = color.g <= 0.04045f ? x.g : y.g;
    clr.b = color.b <= 0.04045f ? x.b : y.b;
    return clr;
#endif
}
