cmake_minimum_required(VERSION 3.3)
project(Sol)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES
	src/stream/streamer.cpp src/stream/streamer.hpp
    src/service_resource.hpp
    src/service_window.hpp
    src/system.hpp
)

add_library(Sol STATIC ${SOURCE_FILES})
