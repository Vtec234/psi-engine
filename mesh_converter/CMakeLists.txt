cmake_minimum_required(VERSION 3.3)
project(Mesh\ Converter)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wextra -Wno-unused -g")

set(SOURCE_FILES
	src/main.cpp
)

include_directories(../psi/src/impl/rendering)
add_executable(mconv ${SOURCE_FILES})
target_link_libraries(mconv boost_filesystem boost_system boost_program_options assimp crypto psi)
