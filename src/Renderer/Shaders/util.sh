#ifndef UTIL_SH_HEADER_GUARD
#define UTIL_SH_HEADER_GUARD

#include <bgfx_shader.sh>

// added in a later bgfx version
// https://github.com/bkaradzic/bgfx/commit/a133fb11c6de8b7aa3c47bdfe405294f2136d4fb
mat3 mtxFromCols(vec3 c0, vec3 c1, vec3 c2)
{
#if BGFX_SHADER_LANGUAGE_GLSL
	return mat3(c0, c1, c2);
#else
	return transpose(mat3(c0, c1, c2));
#endif
}

// from screen coordinates (gl_FragCoord) to eye space
vec4 screen2Eye(vec4 coord)
{
#if BGFX_SHADER_LANGUAGE_GLSL
    // https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
    vec4 ndc = vec4(
        2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
        2.0 * (coord.y - u_viewRect.y) / u_viewRect.w - 1.0,
        (2.0 * coord.z - 1.0), // -> [-1, 1]
        1.0
    );
#else
    vec4 ndc = vec4(
        2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
        (2.0 * (u_viewRect.w - coord.y - 1 - u_viewRect.y) / u_viewRect.w) - 1.0, // y is flipped
        coord.z, // -> [0, 1]
        1.0
    );
#endif

    vec4 eye = mul(u_invProj, vec4(ndc.xyz, 1.0));
    eye = eye / eye.w;

    return eye;
}

#endif // UTIL_SH_HEADER_GUARD
