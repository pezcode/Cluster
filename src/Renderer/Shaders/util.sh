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
    vec3 ndc = vec3(
        2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
        2.0 * (coord.y - u_viewRect.y) / u_viewRect.w - 1.0,
        2.0 * coord.z - 1.0 // -> [-1, 1]
    );
#else
    vec3 ndc = vec3(
        2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
        2.0 * (u_viewRect.w - coord.y - 1 - u_viewRect.y) / u_viewRect.w - 1.0, // y is flipped
        coord.z // -> [0, 1]
    );
#endif

    // https://stackoverflow.com/a/16597492/862300
    vec4 eye = mul(u_invProj, vec4(ndc, 1.0));
    eye = eye / eye.w;

    return eye;
}

// depth from screen coordinates (gl_FragCoord.z) to eye space
// same as screen2Eye(vec4(0, 0, depth, 1)).z but slightly faster
// (!) this assumes a perspective projection as created by bx::mtxProj
// for a left-handed coordinate system
float screen2EyeDepth(float depth, float near, float far)
{
    // https://stackoverflow.com/a/45710371/862300

#if BGFX_SHADER_LANGUAGE_GLSL
    float ndc = 2.0 * depth - 1.0;
    // ndc = (eye * (far + near) / (far - near) - 2 * (far * near) / (far - near)) / eye
    float eye = 2.0 * far * near / (far + near + ndc * (near - far));
#else
    float ndc = depth;
    // ndc = (eye * far / (far - near) - (far * near) / (far - near)) / eye
    float eye = far * near / (far + ndc * (near - far));
#endif

    return eye;
}

#endif // UTIL_SH_HEADER_GUARD
