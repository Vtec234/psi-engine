cmake_minimum_required(VERSION 3.3)
project(Psi\ Engine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wextra -Wno-unused -g")

set(SOURCE_FILES
	src/impl/resource/data.hpp
	src/impl/resource/file.cpp src/impl/resource/file.hpp
	src/impl/resource/glsl.cpp src/impl/resource/glsl.hpp
	src/impl/scene/default_components.hpp
	src/impl/service/resource.cpp src/impl/service/resource.hpp
	src/impl/service/window_gl.cpp src/impl/service/window_gl.hpp
	src/impl/system/renderer_gl.cpp src/impl/system/renderer_gl.hpp
	src/impl/impl.hpp
	src/log/log.cpp src/log/log.hpp
	src/marker/thread_safety.hpp
	src/scene/access.hpp
	src/scene/components.hpp
	src/service/manager.cpp src/service/manager.hpp
	src/service/resource.hpp
	src/service/window.hpp
	src/system/manager.cpp src/system/manager.hpp
	src/system/system.hpp
	src/thread/manager.cpp src/thread/manager.hpp
	src/util/assert.hpp
	src/util/gl.cpp src/util/gl.hpp
	src/util/stream.cpp src/util/stream.hpp
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GL REQUIRED gl)

add_library(psi STATIC ${SOURCE_FILES})
target_link_libraries(psi boost_filesystem boost_system ${GL_LIBRARIES} glfw tbb)
