# Main executable
add_executable(rhyme-and-meter main.cpp)
target_link_libraries(rhyme-and-meter rhyme_and_meter_lib)

# Calibration utility
add_executable(phonetic-calibration phonetic_calibration.cpp)
target_link_libraries(phonetic-calibration rhyme_and_meter_lib)
