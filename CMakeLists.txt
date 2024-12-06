cmake_minimum_required(VERSION 3.14)
project(rhyme_and_meter)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# # START EMSCRIPTEN ADD

#   # Ensure we're using Emscripten compilers

#   set(CMAKE_C_COMPILER emcc)
#   set(CMAKE_CXX_COMPILER em++)


#   # Specify the Emscripten toolchain file BEFORE the project() call
#   set(CMAKE_TOOLCHAIN_FILE "/usr/local/Cellar/emscripten/3.1.61/libexec/cmake/Modules/Platform/Emscripten.cmake")

#   # # Set optimization flags for Release builds
#   # set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#   set(CMAKE_CXX_FLAGS "-O3") # Optimize for performance


#   # # Ensure CMake uses the Release configuration
#   # set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

#   add_executable(rhyme-and-meter src/main.cpp src/CMU_Dict.cpp src/VowelHexGraph.cpp)

#   # Include headers
#   target_include_directories(rhyme-and-meter PUBLIC ${CMAKE_SOURCE_DIR}/include)

#   # Add Emscripten-specific flags to output WASM
#   set_target_properties(rhyme-and-meter PROPERTIES
#       LINK_FLAGS "-fexceptions -lembind -s INVOKE_RUN=0 -s MODULARIZE=1 --preload-file ${CMAKE_SOURCE_DIR}/data/cmudict-0.7b@/data/cmudict-0.7b -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]'"
#   )

# # END EMSCRIPTEN ADD


# START NORMAL TESTING BUILD

# FetchContent setup to download Catch2
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)

FetchContent_MakeAvailable(Catch2)

# Main application
add_subdirectory(src)

# Enable testing
enable_testing()

# Test directory
add_subdirectory(tests)

# END NORMAL TESTING BUILD
