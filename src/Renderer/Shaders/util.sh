#ifndef UTIL_SH_HEADER_GUARD
#define UTIL_SH_HEADER_GUARD

mat3 mtx3FromCols(vec3 c0, vec3 c1, vec3 c2)
{
#if BGFX_SHADER_LANGUAGE_GLSL
	return mat3(c0, c1, c2);
#else
	return transpose(mat3(c0, c1, c2));
#endif
}

#endif // UTIL_SH_HEADER_GUARD
