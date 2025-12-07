# V0 to V1 Migration Guide

## Overview

This repository now supports building both v0 (original) and v1 (refactored) versions side-by-side during the refactoring process.

## Directory Structure

```
rhyme-and-meter/
├── v0/                      # Original stable version
│   ├── include/            # Original headers
│   ├── src/                # Original implementations
│   ├── tests/              # Original tests
│   ├── CMakeLists.txt      # V0 build configuration
│   └── README.md           # V0 documentation
├── v1/                      # Refactored version (work in progress)
│   ├── include/            # Refactored headers
│   ├── src/                # Refactored implementations
│   ├── tests/              # Refactored tests
│   ├── CMakeLists.txt      # V1 build configuration
│   └── README.md           # V1 documentation
├── data/                    # Shared data (CMU Dictionary)
├── CMakeLists.txt          # Root orchestrator
└── CMakePresets.json       # Build presets for both versions
```

## Build Presets

### V0 Presets (Original)
- `v0-debug` - Debug build of original version
- `v0-release` - Release build of original version
- `emscripten-v0` - WebAssembly build of original version

### V1 Presets (Refactored)
- `v1-debug` - Debug build of refactored version
- `v1-release` - Release build of refactored version

### Combined
- `both-debug` - Build both v0 and v1 in debug mode

## Quick Commands

### Working with V0
```bash
# Configure
cmake --preset v0-debug

# Build
cmake --build --preset v0-debug

# Test
ctest --preset v0-debug

# Build executables location
build/v0-debug/v0/src/rhyme-and-meter-v0
build/v0-debug/v0/src/phonetic-calibration-v0
```

### Working with V1
```bash
# Configure
cmake --preset v1-debug

# Build
cmake --build --preset v1-debug

# Test
ctest --preset v1-debug
```

### Emscripten (V0)
```bash
emcmake cmake --preset emscripten-v0
emmake cmake --build --preset emscripten-v0
# Output: build/emscripten-v0/
```

## Migration Workflow

1. **Start with a file from v0**
   - Example: `v0/src/phonetic.cpp`

2. **Copy to v1**
   ```bash
   cp v0/include/phonetic.hpp v1/include/
   cp v0/src/phonetic.cpp v1/src/
   cp v0/tests/test_phonetic.cpp v1/tests/
   ```

3. **Refactor the v1 version**
   - Update the implementation
   - Modify the API as needed
   - Update tests

4. **Update v1/CMakeLists.txt**
   - Add the new source files
   - Link libraries appropriately

5. **Test both versions**
   ```bash
   ctest --preset v0-debug  # Ensure v0 still works
   ctest --preset v1-debug  # Test v1 changes
   ```

6. **Repeat until all files are migrated**

7. **Clean up**
   - Once v1 is complete and tested, remove `v0/`
   - Update presets to remove v0
   - Move v1 to root (optional)

## IntelliSense Setup

The `compile_commands.json` symlink points to v0 by default:
```bash
compile_commands.json -> build/v0-debug/compile_commands.json
```

To switch to v1 for IntelliSense:
```bash
ln -sf build/v1-debug/compile_commands.json compile_commands.json
```

## Target Names

All targets have version suffixes to avoid conflicts:

### V0 Targets
- `phonetic_v0` - Phonetic library
- `rhyme_and_meter_lib_v0` - Core library
- `rhyme-and-meter-v0` - Main executable
- `phonetic-calibration-v0` - Calibration utility
- `tests_v0` - Main tests
- `test_phonetic_v0` - Phonetic tests

### V1 Targets
- `phonetic_v1` - Phonetic library (when added)
- `rhyme_and_meter_lib_v1` - Core library (when added)
- etc.

## Testing Both Versions

Run both test suites:
```bash
cmake --preset both-debug
cmake --build --preset both-debug
ctest --preset both-debug
```

This ensures your refactoring doesn't break the original implementation.

## Notes

- Keep v0 working at all times - it's your safety net
- v1 can be completely broken during development
- Tests are your friend - migrate them along with the code
- Use the `both-debug` preset regularly to catch regressions
