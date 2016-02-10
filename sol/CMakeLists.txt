cmake_minimum_required(VERSION 3.3)
project(Sol)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -g")

set(SOURCE_FILES
	src/io/streamer.cpp src/io/streamer.hpp
	src/scene/system.hpp
	src/service/resource.hpp
	src/service/window.hpp
	src/task/submitter.hpp
)

add_library(Sol STATIC ${SOURCE_FILES})
