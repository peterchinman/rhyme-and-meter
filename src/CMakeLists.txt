add_executable(rhyme-and-meter main.cpp CMU_Dict.cpp)

# Include directories
target_include_directories(rhyme-and-meter PUBLIC ${CMAKE_SOURCE_DIR}/include)
