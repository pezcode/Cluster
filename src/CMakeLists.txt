set(SOURCES
	main.cpp
	Cluster.h
	Cluster.cpp
	Config.h
	Config.cpp
	UI.h
	UI.cpp
	Scene/Scene.h
	Scene/Scene.cpp
	Scene/Camera.h
	Scene/Camera.cpp
	Scene/Mesh.h
	Scene/Mesh.cpp
	Scene/Material.h
	Scene/Light.h
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
	Log/Log.h
	Log/Log.cpp
	Log/UISink.h
	Log/AssimpSource.h
	vs_forward.sc
	fs_forward.sc
	vs_tonemap.sc
	fs_tonemap.sc
	tonemapping.sh
	pbr_shader.sh
	varying.def.sc
)

add_executable(Cluster ${SOURCES})
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

add_shader(vs_forward.sc VERTEX   OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)
add_shader(fs_forward.sc FRAGMENT OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)
add_shader(vs_tonemap.sc VERTEX   OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)
add_shader(fs_tonemap.sc FRAGMENT OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)

file(COPY ../assets/ DESTINATION ${ASSETS_DIR})

install(TARGETS Cluster RUNTIME DESTINATION bin)
install(DIRECTORY ${SHADER_DIR} DESTINATION bin)
install(DIRECTORY ${ASSETS_DIR} DESTINATION bin)
