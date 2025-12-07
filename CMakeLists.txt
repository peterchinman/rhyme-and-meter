cmake_minimum_required(VERSION 3.15)
project(rhyme_and_meter)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Option to build v0, v1, or both
option(BUILD_V0 "Build v0 (original version)" ON)
option(BUILD_V1 "Build v1 (refactored version)" OFF)

message(STATUS "===========================================")
message(STATUS "Rhyme and Meter - Multi-version Build")
message(STATUS "===========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build v0: ${BUILD_V0}")
message(STATUS "Build v1: ${BUILD_V1}")
message(STATUS "===========================================")

# Detect Emscripten
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(EMSCRIPTEN ON)
    message(STATUS "Emscripten build detected")
else()
    set(EMSCRIPTEN OFF)
    message(STATUS "Native build detected")
endif()

# FetchContent setup for Catch2 (only for native builds)
if(NOT EMSCRIPTEN)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
    
    # Enable testing
    enable_testing()
endif()

# Build v0 if requested
if(BUILD_V0)
    message(STATUS "")
    message(STATUS "Building V0 (original)...")
    add_subdirectory(v0)
endif()

# Build v1 if requested
if(BUILD_V1)
    message(STATUS "")
    message(STATUS "Building V1 (refactored)...")
    add_subdirectory(v1)
endif()

message(STATUS "")
message(STATUS "Configuration complete!")
message(STATUS "===========================================")
