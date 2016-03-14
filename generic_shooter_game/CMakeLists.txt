cmake_minimum_required(VERSION 3.3)
project(Generic\ Shooter\ Game)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -g -Wall")

set(SOURCE_FILES
	src/data/parse.cpp src/data/parse.hpp
	src/gl/gl.cpp src/gl/gl.hpp
	src/log/log.cpp src/log/log.hpp
    src/manager/environment.cpp src/manager/environment.hpp
	src/manager/service.cpp src/manager/service.hpp
	src/manager/system.cpp src/manager/system.hpp
	src/resource/glsl_source.cpp src/resource/glsl_source.hpp
	src/service/resource.cpp src/service/resource.hpp
	src/service/window_gl.cpp src/service/window_gl.hpp
	src/system/renderer_gl.cpp src/system/renderer_gl.hpp
	src/task/manager.cpp src/task/manager.hpp
    src/main.cpp
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GL REQUIRED gl)

include_directories(../sol/src)
add_executable(GSG ${SOURCE_FILES})
target_link_libraries(GSG boost_program_options boost_filesystem boost_system glfw tbb Sol ${GL_LIBRARIES})
add_definitions(-D_DEBUG)
