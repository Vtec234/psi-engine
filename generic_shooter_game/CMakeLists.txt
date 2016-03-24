cmake_minimum_required(VERSION 3.3)
project(Generic\ Shooter\ Game)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wextra -Wno-unused -g")

set(SOURCE_FILES
	src/env/environment.cpp src/env/environment.hpp
    src/main.cpp
)

include_directories(../psi/src)

add_executable(gsg ${SOURCE_FILES})
target_link_libraries(gsg boost_program_options psi)
