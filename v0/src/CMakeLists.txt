# V0 Main executables

# Main executable
add_executable(rhyme-and-meter-v0 main.cpp)
target_link_libraries(rhyme-and-meter-v0 rhyme_and_meter_lib_v0)

# Calibration utility
add_executable(phonetic-calibration-v0 phonetic_calibration.cpp)
target_link_libraries(phonetic-calibration-v0 rhyme_and_meter_lib_v0)
