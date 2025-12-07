# Rhyme and Meter

A collection of C++ methods for checking validating poetic rhmye and meter.

## Project Structure

This repository contains both:
- **v0**: The original stable version (in `v0/`)
- **v1**: The refactored version (in `v1/`) - work in progress

During the refactoring period, both versions can be built and tested independently.

## Intro

Uses my [Phonetic](https://github.com/peterchinman/phonetic/) library to get pronunciations using the [CMU Pronouncing Dictionary](http://www.speech.cs.cmu.edu/cgi-bin/cmudict#about). For the rhyme checking, we calcuate a "rhyme distance", using a notion of "vowel distance", "consonant distance" and techniques from bio-informatics to align string sequences.

The meter checking is a minimum validation that a certain meter could match a certain line, without trying to guess what the actual meter of the text is. 

[More information here.](https://peterchinman.com/blog/strictly)

## Build

### Initial Setup

This is a monorepo containing the rhyme-and-meter library, phonetic library, and CMU Pronouncing Dictionary data all in one place.

Run the development setup script:
```bash
./scripts/setup-dev.sh
```

This will:
- Set up pre-commit hooks (auto-runs tests before commits)
- Make all scripts executable
- Check for required dependencies
- Configure the v0 debug build

### Native Build (using CMake Presets)

The project uses modern CMake presets for standardized builds. You can build either v0, v1, or both.

**V0 (original) debug build:**
```bash
cmake --preset v0-debug
cmake --build --preset v0-debug
ctest --preset v0-debug
```

**V0 (original) release build:**
```bash
cmake --preset v0-release
cmake --build --preset v0-release
ctest --preset v0-release
```

**V1 (refactored) debug build:**
```bash
cmake --preset v1-debug
cmake --build --preset v1-debug
ctest --preset v1-debug
```

**Build both v0 and v1 together:**
```bash
cmake --preset both-debug
cmake --build --preset both-debug
ctest --preset both-debug
```

**Quick test script:**
```bash
./scripts/test-quick.sh          # Uses debug preset
./scripts/test-quick.sh release  # Uses release preset
```

**Watch mode (auto-run tests on file changes):**
```bash
./scripts/watch-tests.sh         # Requires: brew install fswatch
```

This will build the test executable and main applications. See `scripts/README.md` for more workflow details.

### WebAssembly Build (Emscripten)

Compile v0 to WebAssembly using [Emscripten](https://emscripten.org/docs/getting_started/downloads.html):

**Configure:**
```bash
emcmake cmake --preset emscripten-v0
```

**Build:**
```bash
emmake cmake --build --preset emscripten-v0
```

Output files will be in `build/emscripten-v0/`.

This will generate `rhyme-and-meter.js`, `rhyme-and-meter.wasm`, and `rhyme-and-meter.data`. Using these you'll be able to call these methods directly from Javascript:
- `Rhyme_and_Meter::check_syllable_validity()`
- `Rhyme_and_Meter::check_meter_validity()`
- `Rhyme_and_Meter::get_end_rhyme_distance()`
- `levenshtein_distance()`

(Note: `rhyme-and-meter.data` expects to be found in the root folder of your web project.)

**JavaScript usage example:**

```javascript
// Initialize the WebAssembly module
Module().then((Module) => {
    console.log("module initialized")

    // Create an instance of Rhyme_and_Meter class
    rhyme_and_meter = new Module.Rhyme_and_Meter();
    
    const result_object = rhyme_and_meter.get_end_rhyme_distance(string1, string2);

    if (result.is_valid) {
        //blah blah blah
    }
    
    // Or, for functions not in a class:
    const distance = Module.levenshtein_distance(string1, string2);
})
```

### Python Bindings

Thanks to @nwoods, this project also has Python bindings. From the project root directory run:

```bash
pip install .
```

You'll then be able to run all of the class methods from inside the tidy confines of Python.

## Usage

See header files for documentation of each method. See tests for methods in action.


### `Rhyme_and_Meter::check_syllable_validity()`

Checks whether a line of text could match a syllable count.

Returns a `Check_Validity_Result` object, containing a bool `is_valid` and a vector of unrecognized words.

### `Rhyme_and_Meter::check_meter_validity()`

Checks whether a line of text could match a meter, given as a string of 'x' and '/', where 'x' is an unstessed syllable and '/' is a stressed syllable. E.g. iambic penatmeter would be rendered as "x/x/x/x/x/".

Returns a `Check_Validity_Result` object, containing a bool `is_valid` and a vector of unrecognized words.

###  `Rhyme_and_Meter::get_end_rhyme_distance()`

Compares the end rhymes of two lines and returns the "rhyme distance". Currently, this takes as its comparison unit the at the shortest rhyming-part from between the last word of the two lines.


### `levenshtein_distance()`

Compares two space-separated strings of CMUdict arpabet phones. 



