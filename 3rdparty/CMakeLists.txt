# bigg (bgfx + imgui + glfw + glm)

add_definitions(-DBGFX_CONFIG_RENDERER_OPENGL_MIN_VERSION=43)
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
set(BIGG_EXAMPLES OFF)
#set(BGFX_BUILD_EXAMPLES OFF)
add_subdirectory(bigg)

# icon font headers

add_library(IconFontCppHeaders INTERFACE)
target_include_directories(IconFontCppHeaders INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/IconFontCppHeaders")

# assimp

set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
add_subdirectory(assimp)

# spdlog

add_subdirectory(spdlog)
