# Rhyme and Meter

A collection of C++ methods for checking validating poetic rhmye and meter. Compiled to WebAssembly and used in [Strictly](https://github.com/peterchinman/strictly), my rhyme and meter checker.

## Intro

Uses my [Phonetic](https://github.com/peterchinman/phonetic/) library to get pronunciations using the [CMU Pronouncing Dictionary](http://www.speech.cs.cmu.edu/cgi-bin/cmudict#about). For the rhyme checking, we calcuate a "rhyme distance", using a notion of "vowel distance" and techniques from bio-informatics to align string sequences. The meter checking is a minimum validation that a certain meter could match a certain line, without trying to guess what the actual meter of the text is. [More information here](https://peterchinman.com/blog/strictly).

## Build

Includes my Phonetic library as a sub-module. Use:
```
git submodule update --init --recursive
```
to load load that library and CMU Pronouncing Dictionary data. 


You can use CMake to compile:

```
mkdir build
cd build
cmake ..
make
```

This will also build  `tests/tests` Catch-2 test file.

This can also be easily compiled to WebAssembly using [Emscripten](https://emscripten.org/docs/getting_started/downloads.html):

```
mdkir build
cd build
emcmake cmake ..
emmake make
```

This will generate `rhyme-and-meter.js`, `rhyme-and-meter.wasm`, and `rhmye-and-meter.data`. Using these you'll be able to call these methods directly from Javascript:
- `Rhyme_and_Meter::check_syllable_validity()`
- `Rhyme_and_Meter::check_meter_validity()`
- `Rhyme_and_Meter::get_end_rhyme_distance()`


(Note:`rhmye-and-meter.data` expects to be found in the root folder of your web project.)

## Usage

See header files for documentation of each method. See tests for methods in action.


### `Rhyme_and_Meter::check_syllable_validity()`

Checks whether a line of text could match a syllable count.

Returns a `Check_Validity_Result` object, containing a bool `is_valid` and a vector of unrecognized words.

### `Rhyme_and_Meter::check_meter_validity()`

Checks whether a line of text could match a meter, given as a string of 'x' and '/', where 'x' is an unstessed syllable and '/' is a stressed syllable. E.g. iambic penatmeter would be rendered as "x/x/x/x/x/".

Returns a `Check_Validity_Result` object, containing a bool `is_valid` and a vector of unrecognized words.

###  `Rhyme_and_Meter::get_end_rhyme_distance()`

Compares the end rhymes of two lines and returns the "rhyme distance". Currently, this looks at the shortest rhyming-part from between the last word of the two lines. 

Scoring:

    0     => perfect rhyme!
    1-5   => pretty dang close, only vowel sound different!
    6-10  => pretty close, consonant differences
    11-20 => further consonant differences
    20+   => noisy, hard to use for information




