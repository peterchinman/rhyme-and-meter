# Development Scripts

This directory contains helper scripts for development workflow automation.

## Scripts

### `setup-dev.sh`
Initial development environment setup. Run this once after cloning.

```bash
./scripts/setup-dev.sh
```

**What it does:**
- Makes all scripts executable
- Sets up pre-commit hooks
- Checks for required dependencies
- Configures the debug build preset

---

### `test-quick.sh`
Quick test runner for manual testing.

```bash
./scripts/test-quick.sh [preset]
```

**Examples:**
```bash
./scripts/test-quick.sh         # Uses debug preset (default)
./scripts/test-quick.sh release # Uses release preset
```

---

## Git Hooks

### Pre-commit Hook
Automatically runs tests before every commit to catch issues early.

**Location:** `.git/hooks/pre-commit`

**Behavior:**
- Builds the project (debug preset)
- Runs all tests
- Aborts commit if tests fail

**Skip hook temporarily:**
```bash
git commit --no-verify
```

---

## CMake Presets

The project uses CMake presets for standardized builds. See `CMakePresets.json`.

### Available Presets:
- `debug` - Debug build with symbols
- `release` - Optimized production build
- `relwithdebinfo` - Optimized with debug symbols

### Usage:

**Configure:**
```bash
cmake --preset debug
```

**Build:**
```bash
cmake --build --preset debug
```

**Test:**
```bash
ctest --preset debug
```

### One-liner for all steps:
```bash
cmake --preset debug && cmake --build --preset debug && ctest --preset debug
```

---

## Typical Workflows

### First Time Setup:
```bash
./scripts/setup-dev.sh
```

### Active Development (with watch mode):
```bash
./scripts/watch-tests.sh
# Edit files, tests run automatically
```

### Quick Manual Test:
```bash
./scripts/test-quick.sh
```

### Before Committing:
Tests run automatically via pre-commit hook, but you can run manually:
```bash
./scripts/test-quick.sh
git add .
git commit -m "Your message"
```

### Release Build:
```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

