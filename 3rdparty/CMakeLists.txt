# bigg (bgfx + imgui + glfw + glm)

add_definitions(-DBGFX_CONFIG_RENDERER_OPENGL_MIN_VERSION=43)
set(BIGG_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory(bigg)

# icon font headers

add_library(IconFontCppHeaders INTERFACE)
target_include_directories(IconFontCppHeaders INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/IconFontCppHeaders")

# assimp

set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "")
set(ASSIMP_BUILD_SAMPLES OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_ZLIB ON CACHE INTERNAL "")
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE INTERNAL "")
add_subdirectory(assimp)

# spdlog

add_subdirectory(spdlog)
