# Add the test executable
add_executable(tests test_rhyme_and_meter.cpp test_vowel_hex_graph.cpp test_consonant_distance.cpp ${CMAKE_SOURCE_DIR}/src/rhyme_and_meter.cpp ${CMAKE_SOURCE_DIR}/src/vowel_hex_graph.cpp ${CMAKE_SOURCE_DIR}/src/consonant_distance.cpp)

target_link_libraries(tests phonetic
                        Catch2::Catch2WithMain )
                        
# Include directories for the test files
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

# # Set working directory
# set_target_properties(tests PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Register tests with CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
