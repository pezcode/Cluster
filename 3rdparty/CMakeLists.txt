set(BIGG_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory(bigg)

add_library(IconFontCppHeaders INTERFACE)
target_include_directories(IconFontCppHeaders INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/IconFontCppHeaders")
