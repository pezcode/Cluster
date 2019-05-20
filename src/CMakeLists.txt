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
    
    Scene/Scene.h
    Scene/Scene.cpp
    Scene/Camera.h
    Scene/Camera.cpp
    Scene/Mesh.h
    Scene/Mesh.cpp
    Scene/Material.h
    Scene/Light.h
    Scene/LightList.h
    Scene/LightList.cpp
)

set(SHADERS
    Renderer/Shaders/varying.def.sc
    Renderer/Shaders/vs_forward.sc
    Renderer/Shaders/fs_forward.sc
    Renderer/Shaders/vs_tonemap.sc
    Renderer/Shaders/fs_tonemap.sc
    Renderer/Shaders/tonemapping.sh
    Renderer/Shaders/pbr.sh
    Renderer/Shaders/lights.sh
)

add_executable(Cluster ${SOURCES} ${SHADERS})
target_include_directories(Cluster PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Cluster PRIVATE bigg IconFontCppHeaders assimp spdlog)
target_compile_definitions(Cluster PRIVATE GLM_FORCE_LEFT_HANDED)

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

foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME "${SHADER}" NAME)
    get_filename_component(SHADER_FILE "${SHADER}" ABSOLUTE)
    if(SHADER_NAME MATCHES "^vs_")
        add_shader("${SHADER_FILE}" VERTEX   OUTPUT "${SHADER_DIR}" PLATFORMS dx11 glsl)
    elseif(SHADER_NAME MATCHES "^fs_")
        add_shader("${SHADER_FILE}" FRAGMENT OUTPUT "${SHADER_DIR}" PLATFORMS dx11 glsl)
    elseif(SHADER_NAME MATCHES "^cs_")
        add_shader("${SHADER_FILE}" COMPUTE  OUTPUT "${SHADER_DIR}" PLATFORMS dx11 glsl)
    else()
        # add_shader does this, do it manually for includes/varying.def.sc
        source_group("Shader Files" FILES ${SHADERS})
    endif()
endforeach()

file(COPY ../assets/ DESTINATION ${ASSETS_DIR})

install(TARGETS Cluster RUNTIME DESTINATION bin)
install(DIRECTORY ${SHADER_DIR} DESTINATION bin)
install(DIRECTORY ${ASSETS_DIR} DESTINATION bin)
