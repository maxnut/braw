cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(RUN_TESTS ON)

project(brawc)

if (NOT WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(brawc ${SOURCES})

target_include_directories(brawc PRIVATE
    src
    include
)

install(TARGETS brawc)

if (RUN_TESTS)
function(compile_braw_test braw_file)
    get_filename_component(test_name ${braw_file} NAME_WE)

    set(ASM_DIR "${CMAKE_BINARY_DIR}/testfiles")
    set(ASM_FILE "${ASM_DIR}/${test_name}.asm")

    add_custom_command(
        OUTPUT ${ASM_FILE}
        COMMAND ${CMAKE_COMMAND} -E env BRAW_STDLIB=${CMAKE_SOURCE_DIR}/stdlib ${CMAKE_BINARY_DIR}/bin/brawc ${braw_file} -o ${ASM_DIR}
        DEPENDS brawc ${braw_file}
        COMMENT "Compiling ${braw_file} -> ${ASM_FILE}"
    )

    set(TEST_ASM_FILES ${TEST_ASM_FILES} ${ASM_FILE} PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE BRAW_TEST_FILES tests/*.braw)

set(TEST_ASM_FILES "")

foreach(BRAW_FILE ${BRAW_TEST_FILES})
    compile_braw_test(${BRAW_FILE})
endforeach()

# Function to assemble all .asm files
function(assemble_all_asm_files)
    set(ASM_DIR "${CMAKE_BINARY_DIR}/testfiles")
    file(GLOB ASM_FILES "${ASM_DIR}/*.asm")

    set(TEST_OBJECTS "")

    foreach(ASM_FILE ${ASM_FILES})
        get_filename_component(FILENAME ${ASM_FILE} NAME_WE)
        set(OBJ_FILE "${ASM_DIR}/${FILENAME}.o")

        add_custom_command(
            OUTPUT ${OBJ_FILE}
            COMMAND ${CMAKE_COMMAND} -E env BRAW_STDLIB=${CMAKE_SOURCE_DIR}/stdlib as --64 -g ${ASM_FILE} -o ${OBJ_FILE}
            DEPENDS ${ASM_FILE}
            COMMENT "Assembling ${ASM_FILE} -> ${OBJ_FILE}"
        )

        list(APPEND TEST_OBJECTS ${OBJ_FILE})
    endforeach()

    set(TEST_OBJECTS ${TEST_OBJECTS} PARENT_SCOPE)
endfunction()

assemble_all_asm_files()

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

add_executable(tests ${TEST_SOURCES} ${TEST_OBJECTS})
target_link_libraries(tests PRIVATE GTest::gtest_main)

install(TARGETS tests)

add_test(NAME AllTests COMMAND tests)

add_custom_command(
    TARGET tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env BRAW_STDLIB=${CMAKE_SOURCE_DIR}/stdlib ./tests
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Running test..."
)
endif()

