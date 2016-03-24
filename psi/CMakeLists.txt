cmake_minimum_required(VERSION 3.3)
project(Psi\ Engine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wextra -Wno-unused -g")

set(SOURCE_FILES
	src/log/log.cpp src/log/log.hpp
	src/marker/thread_safety.hpp
	src/scene/impl/default_components.hpp
	src/scene/access.hpp
	src/scene/components.hpp
	src/service/impl/glsl_source.cpp src/service/impl/glsl_source.hpp
	src/service/impl/resource.cpp src/service/impl/resource.hpp
	src/service/impl/window_gl.cpp src/service/impl/window_gl.hpp
	src/service/manager.cpp src/service/manager.hpp
	src/service/resource.hpp
	src/service/window.hpp
	src/system/impl/renderer_gl.cpp src/system/impl/renderer_gl.hpp
	src/system/manager.cpp src/system/manager.hpp
	src/system/system.hpp
	src/thread/impl/manager.cpp src/thread/impl/manager.hpp
	src/thread/task.hpp
	src/util/assert.hpp
	src/util/file.cpp src/util/file.hpp
	src/util/gl.cpp src/util/gl.hpp
	src/util/stream.cpp src/util/stream.hpp
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GL REQUIRED gl)

add_library(psi STATIC ${SOURCE_FILES})
target_link_libraries(psi boost_filesystem boost_system ${GL_LIBRARIES} glfw tbb)
