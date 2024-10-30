add_executable(rhyme-and-meter main.cpp CMU_Dict.cpp)

# # Set working directory
# set_target_properties(rhyme-and-meter PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Include directories
target_include_directories(rhyme-and-meter PUBLIC ${CMAKE_SOURCE_DIR}/include)


