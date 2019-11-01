set(SOURCES
    main.cpp
    Cluster.h
    Cluster.cpp
    Config.h
    Config.cpp
    UI.h
    UI.cpp

    Log/Log.h
    Log/Log.cpp
    Log/UISink.h
    Log/AssimpSource.h

    Renderer/Renderer.h
    Renderer/Renderer.cpp
    Renderer/ForwardRenderer.h
    Renderer/ForwardRenderer.cpp
    Renderer/DeferredRenderer.h
    Renderer/DeferredRenderer.cpp
    Renderer/ClusteredRenderer.h
    Renderer/ClusteredRenderer.cpp
    Renderer/PBRShader.h
    Renderer/PBRShader.cpp
    Renderer/LightShader.h
    Renderer/LightShader.cpp
    Renderer/ClusterShader.h
    Renderer/ClusterShader.cpp
    Renderer/Samplers.h

    Scene/Scene.h
    Scene/Scene.cpp
    Scene/Camera.h
    Scene/Camera.cpp
    Scene/Mesh.h
    Scene/Mesh.cpp
    Scene/Material.h
    Scene/Light.h
    Scene/Light.cpp
    Scene/LightList.h
    Scene/LightList.cpp
)

set(SHADERS
    Renderer/Shaders/varying.def.sc
    Renderer/Shaders/vs_clustered.sc
    Renderer/Shaders/fs_clustered.sc
    Renderer/Shaders/fs_clustered_debug_vis.sc
    Renderer/Shaders/cs_clustered_clusterbuilding.sc
    Renderer/Shaders/cs_clustered_lightculling.sc
    Renderer/Shaders/vs_deferred_geometry.sc
    Renderer/Shaders/fs_deferred_geometry.sc
    Renderer/Shaders/vs_deferred_light.sc
    Renderer/Shaders/fs_deferred_light.sc
    Renderer/Shaders/vs_forward.sc
    Renderer/Shaders/fs_forward.sc
    Renderer/Shaders/vs_tonemap.sc
    Renderer/Shaders/fs_tonemap.sc
    Renderer/Shaders/samplers.sh
    Renderer/Shaders/tonemapping.sh
    Renderer/Shaders/pbr.sh
    Renderer/Shaders/lights.sh
    Renderer/Shaders/clusters.sh
    Renderer/Shaders/colormap.sh
    Renderer/Shaders/util.sh
)

add_executable(Cluster ${SOURCES} ${SHADERS})
target_include_directories(Cluster PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Cluster PRIVATE bigg IconFontCppHeaders assimp spdlog)
target_compile_definitions(Cluster PRIVATE
    # make all types aligned
    # GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    # SIMD optimizations
    GLM_FORCE_INTRINSICS
    # inline code where possible
    GLM_FORCE_INLINE
    # no implicit conversions
    GLM_FORCE_EXPLICIT_CTOR
    # length() returns size_t instead of int
    GLM_FORCE_SIZE_T_LENGTH
    # left-handed projection matrices
    GLM_FORCE_LEFT_HANDED
)

set_target_properties(Cluster PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)

configure_debugging(Cluster WORKING_DIR ${PROJECT_BINARY_DIR})

if(MSVC)
    # disable macro redefinition warning
    # ideally 3rd party include folders were marked as SYSTEM so we wouldn't get these warnings
    target_compile_options(Cluster PRIVATE "/wd4005")
    # hide console window
    target_link_options(Cluster PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
endif()

set(SHADER_DIR "${PROJECT_BINARY_DIR}/shaders")
set(ASSETS_DIR "${PROJECT_BINARY_DIR}/assets")

# target to force all shaders to recompile on the next build
# sometimes handy after changing included header files (.sh)
# since add_shader doesn't take those dependencies into account
add_custom_target(invalidate_shaders)

foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME "${SHADER}" NAME)
    get_filename_component(SHADER_FILE "${SHADER}" ABSOLUTE)
    # add_shader defaults to GLSL 120, 430 for compute
    # 130 is needed for switch statements but requires OpenGL 3.0 or higher
    # need a newer bgfx version that uses BGFX_CONFIG_RENDERER_OPENGL_MIN_VERSION to set
    # OpenGL version without forcing the OpenGL renderer
    # (https://github.com/bkaradzic/bgfx/commit/3f6d8a516d5e61db795c0c1881e9b333fedc5d43)
    # default DX11 model is 5_0
    set(GLSL_VERSION 130)
    set(GLSL_COMPUTE_VERSION 430)
    set(DX_MODEL 5_0)

    if(SHADER_NAME MATCHES "^vs_")
        add_shader("${SHADER_FILE}" VERTEX   OUTPUT "${SHADER_DIR}" GLSL_VERSION ${GLSL_VERSION} DX11_MODEL ${DX_MODEL} PLATFORMS dx11 glsl)
    elseif(SHADER_NAME MATCHES "^fs_")
        add_shader("${SHADER_FILE}" FRAGMENT OUTPUT "${SHADER_DIR}" GLSL_VERSION ${GLSL_VERSION} DX11_MODEL ${DX_MODEL} PLATFORMS dx11 glsl)
    elseif(SHADER_NAME MATCHES "^cs_")
        add_shader("${SHADER_FILE}" COMPUTE  OUTPUT "${SHADER_DIR}" GLSL_VERSION ${GLSL_COMPUTE_VERSION} DX11_MODEL ${DX_MODEL} PLATFORMS dx11 glsl)
    endif()
    add_custom_command(TARGET invalidate_shaders PRE_BUILD
        COMMAND "${CMAKE_COMMAND}" -E touch "${SHADER_FILE}")
endforeach()

# add_shader does this, do it manually for includes/varying.def.sc
source_group("Shader Files" FILES ${SHADERS})

file(COPY ../assets/ DESTINATION ${ASSETS_DIR})

install(TARGETS Cluster RUNTIME DESTINATION bin)
install(DIRECTORY ${SHADER_DIR} DESTINATION bin)
install(DIRECTORY ${ASSETS_DIR} DESTINATION bin)
