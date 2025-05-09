cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(RUN_TESTS ON)

project(brawc)

if (NOT WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fexperimental-library")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(brawc ${SOURCES})

target_include_directories(brawc PRIVATE
    src
    include
)

find_package(TBB REQUIRED)
target_link_libraries(brawc PRIVATE TBB::tbb)

install(TARGETS brawc)

if (RUN_TESTS)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/a.out
    COMMAND ${CMAKE_COMMAND} -E env
            BRAW_STDLIB=${CMAKE_SOURCE_DIR}/stdlib
            ${CMAKE_BINARY_DIR}/bin/brawc
            ${CMAKE_SOURCE_DIR}/tests/test.braw
            -o ${CMAKE_BINARY_DIR}/build_test --assemble --link
            -O 1 --parallel
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building test executable"
    DEPENDS brawc
)

add_custom_target(build_test ALL
    DEPENDS ${CMAKE_BINARY_DIR}/a.out
)
add_custom_target(run_test ALL
    COMMAND ./a.out
    DEPENDS build_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests"
)
add_custom_target(delete_test ALL
    COMMAND ${CMAKE_COMMAND} -E rm -f ./a.out
    COMMAND ${CMAKE_COMMAND} -E rm -r ./build_test
    DEPENDS build_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Deleting build files"
)
endif()
