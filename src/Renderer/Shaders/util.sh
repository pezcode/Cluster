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

    // https://stackoverflow.com/a/16597492/862300
    vec4 eye = mul(u_invProj, vec4(ndc.xyz, 1.0));
    eye = eye / eye.w;

    return eye;
}

// depth from screen coordinates (gl_FragCoord.z) to eye space
// same as screen2Eye(vec4(0, 0, depth, 1)).z but slightly faster
// TODO doesn't work for D3D
float screen2EyeDepth(float depth, float near, float far)
{
#if BGFX_SHADER_LANGUAGE_GLSL
    float ndc = 2.0 * depth - 1.0;
#else
    float ndc = depth;
#endif

    // https://stackoverflow.com/a/45710371/862300

    // if we have the near and far plane:
    float eye = (near * far) / ((far - near) * ndc - far);
    //float eye = 2.0 * near * far / (ndc * (far - near) - (far + near));
    //eye = (-eye - near) / (far - near);

    // otherwise, get it from the projection matrix:
    // this doesn't seem to work
    //float A = u_proj[2][2];
    //float B = u_proj[3][2];
    //float eye = B / (A + ndc);

    return eye;
}

#endif // UTIL_SH_HEADER_GUARD
