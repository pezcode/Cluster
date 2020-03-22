#ifndef UTIL_SH_HEADER_GUARD
#define UTIL_SH_HEADER_GUARD

#include <bgfx_shader.sh>

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

// convert normal from tangent space to space of normal_ref and tangent_ref
vec3 convertTangentNormal(vec3 normal_ref, vec3 tangent_ref, vec3 normal)
{
    vec3 bitangent = cross(normal_ref, tangent_ref);
    mat3 TBN = mtxFromCols(
        normalize(tangent_ref),
        normalize(bitangent),
        normalize(normal_ref)
    );
    return normalize(mul(TBN, normal));
}

// compress viewspace normal into two components
// spheremap transform used by Cry Engine 3
// https://aras-p.info/texts/CompactNormalStorage.html#method04spheremap
// must be viewspace so we can hide z-pole
// both (0,0,1) and (0,0,-1) have the same encoding
// the original implementation always decoded to (0,0,1)
// changed it to return normals pointing towards the camera for
// a left-handed coordinate system

vec2 packNormal(vec3 normal)
{
    float f = sqrt(8.0 * -normal.z + 8.0);
    return normal.xy / f + 0.5;
}

vec3 unpackNormal(vec2 encoded)
{
    vec2 fenc = encoded * 4.0 - 2.0;
    float f = dot(fenc, fenc);
    float g = sqrt(1.0 - f * 0.25);
    return vec3(fenc * g, -(1.0 - f * 0.5));
}

#endif // UTIL_SH_HEADER_GUARD
