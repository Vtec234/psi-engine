cmake_minimum_required(VERSION 3.3)
project(Psi\ Engine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wextra -Wno-unused -g")

set(SOURCE_FILES
	src/impl/rendering/gl/gl.cpp src/impl/rendering/gl/gl.hpp
	src/impl/rendering/gl/helper.cpp src/impl/rendering/gl/helper.hpp
	src/impl/rendering/camera.cpp src/impl/rendering/camera.hpp
	src/impl/rendering/resource.cpp src/impl/rendering/resource.hpp
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
	src/util/enum.hpp
	src/util/file.cpp src/util/file.hpp
	src/util/stream.cpp src/util/stream.hpp
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GL REQUIRED gl)

add_library(psi STATIC ${SOURCE_FILES})
target_link_libraries(psi boost_filesystem boost_system ${GL_LIBRARIES} glfw tbb png jpeg tiff)
