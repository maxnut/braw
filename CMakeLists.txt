cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()
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