set(SOURCES
	main.cpp
	Cluster.h
	Cluster.cpp
	Config.h
	Config.cpp
	UI.h
	UI.cpp
	Scene.h
	Scene.cpp
	Camera.h
	Camera.cpp
	Renderer/Renderer.h
	Renderer/Renderer.cpp
	Renderer/ForwardRenderer.h
	Renderer/ForwardRenderer.cpp
	Renderer/DeferredRenderer.h
	Renderer/DeferredRenderer.cpp
	Renderer/ClusteredRenderer.h
	Renderer/ClusteredRenderer.cpp
	Log/Log.h
	Log/Log.cpp
	Log/UISink.h
	Log/AssimpSource.h
	vs_forward.sc
	fs_forward.sc
	vs_screen_quad.sc
	fs_screen_quad.sc
	pbr.sh
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
	# hide console window
	target_link_options(Cluster PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
endif()

set(SHADER_DIR "${PROJECT_BINARY_DIR}/shaders")
set(ASSETS_DIR "${PROJECT_BINARY_DIR}/assets")

add_shader(vs_forward.sc     VERTEX   OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)
add_shader(fs_forward.sc     FRAGMENT OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)
add_shader(vs_screen_quad.sc VERTEX   OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)
add_shader(fs_screen_quad.sc FRAGMENT OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)

file(COPY ../assets/ DESTINATION ${ASSETS_DIR})

install(TARGETS Cluster RUNTIME DESTINATION bin)
install(DIRECTORY ${SHADER_DIR} DESTINATION bin)
install(DIRECTORY ${ASSETS_DIR} DESTINATION bin)
