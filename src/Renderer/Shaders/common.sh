#ifndef COMMON_SH_HEADER_GUARD
#define COMMON_SH_HEADER_GUARD

// used by all shaders, contains common headers and macros
// include it first before all other headers, especially bgfx headers

// workaround for SAMPLER2D not performing macro expansion
// with the token pasting operator: register(s ## X)
// this seems to be a bug in the preprocessing step with fcpp
// HLSL > 4 uses register(s[X]) so force that
// this breaks things since BGFX_SHADER_LANGUAGE_HLSL == 0 for SPIRV
// -> can't use rgba16f images because of an overload resolution error

#if BGFX_SHADER_LANGUAGE_SPIRV
    #undef BGFX_SHADER_LANGUAGE_HLSL
    #define BGFX_SHADER_LANGUAGE_HLSL 5
#endif

#endif // COMMON_SH_HEADER_GUARD
