# V1 Tests - Placeholder

# Placeholder test executable
add_executable(tests_v1
    test_placeholder.cpp
)

target_link_libraries(tests_v1
    Catch2::Catch2WithMain
)

# Register tests with CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests_v1)
