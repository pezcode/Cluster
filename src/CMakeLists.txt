set(SOURCES
	main.cpp
	Cluster.h
	Cluster.cpp
	vs_cubes.sc
	fs_cubes.sc
)

add_executable(Cluster ${SOURCES})
target_include_directories(Cluster PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Cluster PRIVATE bigg)

if(MSVC)
	# hide console window
	target_link_options(Cluster PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
endif()

set(SHADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

add_shader(vs_cubes.sc VERTEX   OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)
add_shader(fs_cubes.sc FRAGMENT OUTPUT ${SHADER_DIR} DX11_MODEL 5_0 GLSL_VERSION 130)

configure_debugging(Cluster WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS Cluster RUNTIME DESTINATION bin)
install(DIRECTORY ${SHADER_DIR} DESTINATION bin)
