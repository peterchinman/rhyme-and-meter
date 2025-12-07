# V1 CMakeLists.txt - Refactored implementation
cmake_minimum_required(VERSION 3.15)

# Compiler warnings
set(COMMON_WARNINGS -Wall -Wextra -pedantic)

message(STATUS "V1: Building refactored version")

# TODO: Add v1 libraries and executables here as you refactor
# Example structure:
#
# add_library(phonetic_v1 src/phonetic.cpp)
# target_include_directories(phonetic_v1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# ...

# Placeholder test to allow v1 to build and pass tests during refactoring
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    add_subdirectory(tests)
endif()

message(STATUS "V1: Add more source files as you refactor from v0")
