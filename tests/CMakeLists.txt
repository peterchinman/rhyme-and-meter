# Main library tests
add_executable(tests 
    test_rhyme_and_meter.cpp 
    test_vowel_hex_graph.cpp 
    test_consonant_distance.cpp 
    test_convenience.cpp
)

target_link_libraries(tests 
    rhyme_and_meter_lib
    Catch2::Catch2WithMain
)

# Phonetic library tests
add_executable(test_phonetic
    test_phonetic.cpp
)

target_link_libraries(test_phonetic
    phonetic
    Catch2::Catch2WithMain
)

# Register tests with CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
catch_discover_tests(test_phonetic)
