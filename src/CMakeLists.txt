add_executable(rhyme-and-meter main.cpp rhyme_and_meter.cpp vowel_hex_graph.cpp consonant_distance.cpp)
add_executable(phonetic-calibration phonetic_calibration.cpp rhyme_and_meter.cpp vowel_hex_graph.cpp consonant_distance.cpp)

target_link_libraries(rhyme-and-meter phonetic)
target_link_libraries(phonetic-calibration phonetic)


# Include directories
target_include_directories(rhyme-and-meter PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(phonetic-calibration PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(rhyme-and-meter PRIVATE 
    -Wall 
    -Wextra 
    -pedantic 
)

target_compile_options(phonetic-calibration PRIVATE 
    -Wall 
    -Wextra 
    -pedantic 
)


