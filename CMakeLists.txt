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

# URL of the LIEF repo (Can be your fork)
set(LIEF_GIT_URL "https://github.com/lief-project/LIEF.git")

# LIEF's version to be used (can be 'main')
set(LIEF_VERSION 0.13.0)

include(FetchContent)
FetchContent_Declare(LIEF
  GIT_REPOSITORY  "${LIEF_GIT_URL}"
  GIT_TAG         ${LIEF_VERSION}
  # You may specify an existing LIEF source directory if you don't want to
  # download. Just comment out the above ``GIT_*`` commands and uncoment the
  # following ``SOURCE_DIR`` line
  #SOURCE_DIR      "${CMAKE_CURRENT_LIST_DIR}/../../.."
  )

if(${CMAKE_VERSION} VERSION_LESS "3.14.0")
  # CMake 3.11 to 3.13 needs more verbose method to make LIEF available
  FetchContent_GetProperties(LIEF)
  if(NOT LIEF_POPULATED)
    FetchContent_Populate(LIEF)
    add_subdirectory(${LIEF_SOURCE_DIR} ${LIEF_BINARY_DIR})
  endif()
else()
  # CMake 3.14+ has single function to make LIEF available (recommended)
  FetchContent_MakeAvailable(LIEF)
endif()

target_include_directories(braw PRIVATE
src
include
)

install(TARGETS braw)