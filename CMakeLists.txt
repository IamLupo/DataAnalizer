cmake_minimum_required(VERSION 3.16.3)

option(JSONCPP_WITH_TESTS "Compile and (for jsoncpp_check) run JsonCpp test executables" OFF)
option(BUILD_SHARED_LIBS "Build jsoncpp_lib as a shared library." ON)
option(BUILD_STATIC_LIBS "Build jsoncpp_lib as a static library." ON)
option(BUILD_OBJECT_LIBS "Build jsoncpp_lib as a object library." OFF)

add_subdirectory(third-party/jsoncpp)
add_subdirectory(third-party/poppler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -s")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")

## DataAnalizer
project(DataAnalizer VERSION 1.0.0)

## Executable
add_executable(DataAnalizer
	src/main.cpp
)

include_directories(DataAnalizer src)
target_link_libraries(DataAnalizer jsoncpp_static)

## Version
execute_process(
    COMMAND git rev-parse --show-toplevel
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_GIT_TOPLEVEL
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND basename "${PROJECT_GIT_TOPLEVEL}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_GIT_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_GIT_BRANCH_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND date
    OUTPUT_VARIABLE PROJECT_COMPILE_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(src/version.h.in ${CMAKE_SOURCE_DIR}/src/version.h @ONLY)

## Installation
install(TARGETS DataAnalizer DESTINATION bin)
