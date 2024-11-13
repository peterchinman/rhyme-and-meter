# Add the test executable
add_executable(tests testCMU_DICT.cpp testVowelHexGraph.cpp ${CMAKE_SOURCE_DIR}/src/CMU_Dict.cpp ${CMAKE_SOURCE_DIR}/src/VowelHexGraph.cpp)


# Link the test executable with Catch2
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

# Include directories for the test files
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

# # Set working directory
# set_target_properties(tests PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Register tests with CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
