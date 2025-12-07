# V1 - Refactored Version

This directory contains the refactored v1 implementation of rhyme-and-meter.

## Status

🚧 Work in progress - gradually migrating from v0

## Structure

- `include/` - Public header files
- `src/` - Implementation files
- `tests/` - Test files

## Building

```bash
cmake --preset v1-debug
cmake --build --preset v1-debug
ctest --preset v1-debug
```

## Migration Notes

When migrating code from v0 to v1:
1. Copy the file from v0 to v1
2. Refactor according to the v1 design
3. Update tests to match new API
4. Keep v0 working until v1 is complete
