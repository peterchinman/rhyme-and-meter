add_executable(rhyme-and-meter main.cpp rhyme_and_meter.cpp vowel_hex_graph.cpp consonant_distance.cpp)

target_link_libraries(rhyme-and-meter phonetic)


# Include directories
target_include_directories(rhyme-and-meter PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(rhyme-and-meter PRIVATE 
    -Wall 
    -Wextra 
    -pedantic 
)


