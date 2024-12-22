cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(braw)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(braw
${SOURCES}
)

target_include_directories(braw PRIVATE
src
include
)

install(TARGETS braw)