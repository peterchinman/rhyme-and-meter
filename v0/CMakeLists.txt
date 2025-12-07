# V0 CMakeLists.txt - Original implementation
cmake_minimum_required(VERSION 3.15)

# Compiler warnings
set(COMMON_WARNINGS -Wall -Wextra -pedantic)

# Phonetic Library
add_library(phonetic_v0 src/phonetic.cpp)
target_include_directories(phonetic_v0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CMU_DICT_PATH "${CMAKE_SOURCE_DIR}/data/CMUdict/cmudict-0.7b")
target_compile_definitions(phonetic_v0 PUBLIC CMU_DICT_PATH="${CMU_DICT_PATH}")
target_compile_options(phonetic_v0 PRIVATE ${COMMON_WARNINGS})

# Create core library for rhyme_and_meter functionality
add_library(rhyme_and_meter_lib_v0
    src/rhyme_and_meter.cpp
    src/vowel_hex_graph.cpp
    src/consonant_distance.cpp
)
target_include_directories(rhyme_and_meter_lib_v0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rhyme_and_meter_lib_v0 PUBLIC phonetic_v0)
target_compile_options(rhyme_and_meter_lib_v0 PRIVATE ${COMMON_WARNINGS})

# Detect or configure for Emscripten
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(EMSCRIPTEN ON)
    message(STATUS "V0: Emscripten detected. Configuring for WebAssembly build.")
else()
    set(EMSCRIPTEN OFF)
    message(STATUS "V0: Standard compiler detected. Configuring for native build.")
endif()

if(EMSCRIPTEN)
    # Set optimization flags for Release builds
    set(CMAKE_CXX_FLAGS "-O3")

    add_executable(rhyme-and-meter-v0 src/main.cpp)
    target_link_libraries(rhyme-and-meter-v0 rhyme_and_meter_lib_v0)

    # Add Emscripten-specific flags to output WASM
    set_target_properties(rhyme-and-meter-v0 PROPERTIES
        LINK_FLAGS "-fexceptions -lembind -s INVOKE_RUN=0 -s MODULARIZE=1 --preload-file \"${CMU_DICT_PATH}@/data/cmudict-0.7b\" -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]'"
    )
else()
    # Native build - add executables and tests

    # Main application executables
    add_subdirectory(src)

    # Tests
    add_subdirectory(tests)
endif()
