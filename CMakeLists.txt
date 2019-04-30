cmake_minimum_required(VERSION 3.13)

project(cluster CXX)
set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE) 
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

add_subdirectory(3rdparty)
add_subdirectory(src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Cluster)
