cmake_minimum_required(VERSION 3.15)
project(rhyme_and_meter)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adding Phonetic Library, linking to includes, and setting the CMU_DICT_PATH
add_library(phonetic ${CMAKE_SOURCE_DIR}/external/phonetic/src/phonetic.cpp)
target_include_directories(phonetic PUBLIC ${CMAKE_SOURCE_DIR}/external/phonetic/include)
set(CMU_DICT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/phonetic/data/CMUdict/cmudict-0.7b")
target_compile_definitions(phonetic PUBLIC CMU_DICT_PATH="${CMU_DICT_PATH}")

# Detect or configure for Emscripten
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(EMSCRIPTEN ON)
    message(STATUS "Emscripten detected. Configuring for WebAssembly build.")
else()
    set(EMSCRIPTEN OFF)
    message(STATUS "Standard compiler detected. Configuring for native build.")
endif()

if(EMSCRIPTEN)
  # Set optimization flags for Release builds
  set(CMAKE_CXX_FLAGS "-O3")

  add_executable(rhyme-and-meter src/main.cpp src/rhyme_and_meter.cpp src/vowel_hex_graph.cpp src/consonant_distance.cpp)

  target_link_libraries(rhyme-and-meter phonetic)
  # Include headers
  target_include_directories(rhyme-and-meter PUBLIC ${CMAKE_SOURCE_DIR}/include)

  # Add Emscripten-specific flags to output WASM
  set_target_properties(rhyme-and-meter PROPERTIES
    LINK_FLAGS "-fexceptions -lembind -s INVOKE_RUN=0 -s MODULARIZE=1 --preload-file \"${CMU_DICT_PATH}@/data/cmudict-0.7b\" -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]'"
  )
else()
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

  add_subdirectory(external/phonetic/tests)

  # END NORMAL TESTING BUILD
endif()
