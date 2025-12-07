# Rhyme and Meter Library - Architecture Refactoring Plan

## Current Structure (v0)

### Problems with Current Design
- `Phonetic`: Dictionary class that maps words to pronunciations
- `Rhyme_and_Meter`: Monolithic utility class with mixed concerns
  - Contains dictionary internally
  - Stateless methods that require carefully formatted input
  - No domain objects representing poetry concepts
  - Hard to compose operations or cache results
  - User must manually orchestrate multi-step operations

### Pain Points
1. **Poor encapsulation**: Data and operations are separated
2. **Verbose API**: Multi-step process for common tasks
3. **No caching**: Expensive computations repeated
4. **Hard to test**: Must understand entire flow
5. **Not composable**: Can't easily build complex operations

---

## New Structure (v1)

### Target Platform: WebAssembly

**Primary Use Case**: Compile to WASM with Emscripten for use in web applications.

This constraint significantly impacts our architectural decisions:

### WASM-Specific Constraints

#### 1. Binary Size
**Challenge**: WASM bundles download over network - size matters
**Implications**:
- Avoid heavy template instantiation (bloats binary)
- Minimize standard library usage (especially iostream, regex)
- Consider splitting into multiple modules if needed
- Dictionary data should be loadable separately

**Solutions**:
```cpp
// ❌ BAD: Heavy template instantiation
std::function<int(int)> fn = ...;  // Type erasure adds overhead

// ✅ BETTER: Use templates only in hot paths
template<typename Fn>
int apply(Fn&& fn, int x) { return fn(x); }

// ❌ BAD: iostream adds ~100KB
#include <iostream>
std::cout << "error" << std::endl;

// ✅ BETTER: Use lighter alternatives
#include <cstdio>
std::printf("error\n");

// ✅ BEST: Format to string, let JS handle output
std::string format_error(const Error& e);
```

#### 2. C++ Standard Library Support in Emscripten

**Current Status** (as of Emscripten 3.1.50+):
- ✅ C++20: Most features supported (concepts, ranges, coroutines)
- ⚠️ C++23: Partial support
  - ✅ `std::expected` - **Supported!** (via libcxx)
  - ✅ `std::ranges::to` - **Supported!**
  - ⚠️ Some newer features may be missing

**Decision**: Use C++20 as baseline, with C++23 `std::expected` (widely available in Emscripten's libcxx)

**Fallback Strategy**:
```cpp
// Use feature detection
#if __has_include(<expected>) && __cpp_lib_expected >= 202202L
    #include <expected>
    using std::expected;
    using std::unexpected;
#else
    // Provide fallback or use tl::expected library
    #include "external/expected.hpp"
    using tl::expected;
    using tl::unexpected;
#endif
```

#### 3. Memory Management

**Challenge**: WASM has linear memory model, no virtual memory
**Implications**:
- Be explicit about memory usage
- Avoid unnecessary copies
- Dictionary (~10MB) needs careful handling
- Consider memory pooling for frequently created objects

**Solutions**:
```cpp
// Dictionary should be loaded once and shared
class Dictionary {
    static std::unique_ptr<Dictionary> s_instance;
    
public:
    // Explicit initialization (don't hide cost)
    static void initialize(std::string_view data);
    static const Dictionary& instance();
    
    // Allow JS to control when dictionary loads
    static bool is_loaded() { return s_instance != nullptr; }
};

// Use string_view to avoid copies when interfacing with JS
class Line {
    std::string m_text;
public:
    // Accept string_view to avoid copying JS strings
    explicit Line(std::string_view text);
    
    // Return string_view when possible
    std::string_view text() const { return m_text; }
};
```

#### 4. JavaScript Interop with Embind

**Challenge**: Need clean C++/JS boundary
**Implications**:
- Keep API surface simple
- Minimize data crossing boundary
- Return simple types or use Embind's type mapping
- Consider JSON for complex results

**API Design for Embind**:
```cpp
// Embind-friendly interface
#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

namespace web_api {
    // Simple, flat API for JS
    struct ValidationResult {
        bool is_valid;
        std::string summary;
        std::vector<std::string> errors;
        
        // Easily convertible to JS object
    };
    
    class PoemValidator {
    public:
        // Simple types cross boundary efficiently
        void add_line(std::string text);
        void set_rhyme_scheme(std::string pattern);
        void set_meter(std::string pattern);
        
        // Return simple struct
        ValidationResult validate() const;
        
        // Or return JSON string for complex data
        std::string validate_detailed() const;
    };
}

EMSCRIPTEN_BINDINGS(rhyme_and_meter) {
    using namespace web_api;
    
    emscripten::class_<PoemValidator>("PoemValidator")
        .constructor<>()
        .function("addLine", &PoemValidator::add_line)
        .function("setRhymeScheme", &PoemValidator::set_rhyme_scheme)
        .function("setMeter", &PoemValidator::set_meter)
        .function("validate", &PoemValidator::validate);
    
    emscripten::value_object<ValidationResult>("ValidationResult")
        .field("isValid", &ValidationResult::is_valid)
        .field("summary", &ValidationResult::summary)
        .field("errors", &ValidationResult::errors);
}
#endif
```

**Usage from JavaScript**:
```javascript
// In your web app
const validator = new Module.PoemValidator();
validator.addLine("Shall I compare thee to a summer's day?");
validator.addLine("Thou art more lovely and more temperate:");
validator.setRhymeScheme("AA");
validator.setMeter("x/x/x/x/x/");

const result = validator.validate();
if (!result.isValid) {
    console.error(result.summary);
    result.errors.forEach(err => console.log(err));
}
```

#### 5. Loading Strategy

**Challenge**: Dictionary data is large, affects page load time
**Implications**:
- Separate dictionary data from code
- Support lazy/progressive loading
- Allow custom dictionaries (user-provided)

**Loading Strategies**:
```cpp
// Strategy 1: Load dictionary from separate file
class Dictionary {
public:
    // Async-friendly: JS can fetch file and pass data
    static std::expected<void, std::string> load_from_string(
        std::string_view data
    );
    
    // Or load from Emscripten's virtual filesystem
    static std::expected<void, std::string> load_from_file(
        const char* path
    );
};
```

```javascript
// JavaScript side - load dictionary asynchronously
async function initRhymeMeter() {
    // Load WASM module
    const Module = await createRhymeMeterModule();
    
    // Fetch dictionary separately (can be cached by browser)
    const response = await fetch('/data/cmudict.txt');
    const dictData = await response.text();
    
    // Initialize dictionary
    Module.Dictionary.loadFromString(dictData);
    
    return Module;
}

// Or use Emscripten's FS API with preloaded data
Module.preRun = [function() {
    Module.FS_createPreloadedFile('/', 'cmudict.txt', 
        'data/cmudict.txt', true, false);
}];
```

#### 6. Error Handling Across Boundary

**Challenge**: C++ exceptions don't cross to JS cleanly, and we need detailed validation results immediately
**Implications**:
- Avoid exceptions in public API
- Use `std::expected` (already our choice!)
- Return rich validation objects that can cross boundary
- Provide immediate feedback per operation (not just at the end)

**Pattern - Granular Validation Results**:
```cpp
// Rich validation result that crosses boundary easily
struct LineValidationResult {
    bool is_valid;
    std::vector<std::string> unrecognized_words;
    std::vector<int> possible_syllable_counts;
    std::string error_message;  // Human-readable summary
    
    // Helper for JS
    bool has_errors() const { return !is_valid; }
    std::string to_json() const;  // Optional: serialize to JSON
};

struct MeterValidationResult {
    bool is_valid;
    bool matches_meter;
    std::string error_message;
    std::vector<std::string> valid_stress_patterns;  // What the line actually has
    std::string expected_pattern;  // What we're checking against
};

struct RhymeValidationResult {
    bool is_valid;
    std::vector<std::pair<int, int>> non_rhyming_pairs;  // Line indices that should rhyme but don't
    std::vector<int> rhyme_distances;  // Distance for each rhyming pair
    std::string error_message;
};

struct PoemValidationResult {
    bool is_valid;
    std::vector<LineValidationResult> line_results;
    MeterValidationResult meter_result;
    RhymeValidationResult rhyme_result;
    std::string summary;  // Overall summary message
    
    // Convenience methods for JS
    int error_count() const;
    std::vector<std::string> all_errors() const;
};

// Web API with immediate feedback
class PoemValidator {
    std::vector<Line> m_lines;
    std::optional<RhymeScheme> m_rhyme_scheme;
    std::optional<MeterScheme> m_meter_scheme;
    
public:
    // Returns validation result immediately
    LineValidationResult add_line(std::string text) {
        Line line{text};
        
        LineValidationResult result;
        result.is_valid = line.is_valid();
        
        if (!result.is_valid) {
            result.unrecognized_words = line.failed_words();
            result.error_message = format_unrecognized_words(line.failed_words());
        } else {
            // Populate additional info
            auto syllables = line.possible_syllable_counts();
            if (syllables) {
                result.possible_syllable_counts = *syllables;
            }
        }
        
        // Store line even if invalid (user might want to see all errors)
        m_lines.push_back(std::move(line));
        
        return result;
    }
    
    // Validate specific line against meter
    MeterValidationResult validate_line_meter(int line_index) const {
        MeterValidationResult result;
        
        if (line_index >= m_lines.size()) {
            result.is_valid = false;
            result.error_message = "Line index out of range";
            return result;
        }
        
        if (!m_meter_scheme) {
            result.is_valid = false;
            result.error_message = "No meter scheme set";
            return result;
        }
        
        const auto& line = m_lines[line_index];
        const auto& meter = (*m_meter_scheme)[line_index % m_meter_scheme->size()];
        
        result.matches_meter = line.fits_meter(meter);
        result.is_valid = result.matches_meter;
        
        if (!result.matches_meter) {
            auto patterns = line.stress_patterns();
            if (patterns) {
                for (const auto& pattern : *patterns) {
                    std::string pattern_str;
                    for (int stress : pattern) {
                        pattern_str += (stress == 0 ? 'x' : '/');
                    }
                    result.valid_stress_patterns.push_back(pattern_str);
                }
            }
            result.expected_pattern = meter.pattern();
            result.error_message = format_meter_error(result);
        }
        
        return result;
    }
    
    void set_rhyme_scheme(std::string pattern) {
        auto scheme = RhymeScheme::parse(pattern);
        if (scheme) {
            m_rhyme_scheme = *scheme;
        }
    }
    
    void set_meter(std::string pattern) {
        auto meter = Meter::parse(pattern);
        if (meter) {
            // Create uniform meter scheme for all lines
            m_meter_scheme = MeterScheme::uniform(*meter, 100);  // Arbitrary large number
        }
    }
    
    // Full poem validation
    PoemValidationResult validate() const {
        PoemValidationResult result;
        
        // Validate each line
        for (size_t i = 0; i < m_lines.size(); ++i) {
            const auto& line = m_lines[i];
            
            LineValidationResult line_result;
            line_result.is_valid = line.is_valid();
            if (!line_result.is_valid) {
                line_result.unrecognized_words = line.failed_words();
                line_result.error_message = format_unrecognized_words(line.failed_words());
            }
            
            result.line_results.push_back(line_result);
        }
        
        // Validate meter if set
        if (m_meter_scheme) {
            result.meter_result = validate_meter();
        }
        
        // Validate rhyme if set
        if (m_rhyme_scheme) {
            result.rhyme_result = validate_rhyme();
        }
        
        // Overall validity
        result.is_valid = 
            std::all_of(result.line_results.begin(), result.line_results.end(),
                       [](const auto& r) { return r.is_valid; }) &&
            result.meter_result.is_valid &&
            result.rhyme_result.is_valid;
        
        result.summary = generate_summary(result);
        
        return result;
    }
    
    // Get validation for a single line (without adding it)
    static LineValidationResult check_line(std::string text) {
        Line line{text};
        
        LineValidationResult result;
        result.is_valid = line.is_valid();
        
        if (!result.is_valid) {
            result.unrecognized_words = line.failed_words();
            result.error_message = format_unrecognized_words(line.failed_words());
        } else {
            auto syllables = line.possible_syllable_counts();
            if (syllables) {
                result.possible_syllable_counts = *syllables;
            }
        }
        
        return result;
    }
    
    // ... other methods
private:
    MeterValidationResult validate_meter() const;
    RhymeValidationResult validate_rhyme() const;
    std::string generate_summary(const PoemValidationResult&) const;
};
```

**Embind Bindings**:
```cpp
EMSCRIPTEN_BINDINGS(rhyme_and_meter) {
    using namespace emscripten;
    
    // Bind validation result types
    value_object<LineValidationResult>("LineValidationResult")
        .field("isValid", &LineValidationResult::is_valid)
        .field("unrecognizedWords", &LineValidationResult::unrecognized_words)
        .field("possibleSyllableCounts", &LineValidationResult::possible_syllable_counts)
        .field("errorMessage", &LineValidationResult::error_message)
        .field("hasErrors", &LineValidationResult::has_errors);
    
    value_object<MeterValidationResult>("MeterValidationResult")
        .field("isValid", &MeterValidationResult::is_valid)
        .field("matchesMeter", &MeterValidationResult::matches_meter)
        .field("errorMessage", &MeterValidationResult::error_message)
        .field("validStressPatterns", &MeterValidationResult::valid_stress_patterns)
        .field("expectedPattern", &MeterValidationResult::expected_pattern);
    
    value_object<RhymeValidationResult>("RhymeValidationResult")
        .field("isValid", &RhymeValidationResult::is_valid)
        .field("nonRhymingPairs", &RhymeValidationResult::non_rhyming_pairs)
        .field("rhymeDistances", &RhymeValidationResult::rhyme_distances)
        .field("errorMessage", &RhymeValidationResult::error_message);
    
    value_object<PoemValidationResult>("PoemValidationResult")
        .field("isValid", &PoemValidationResult::is_valid)
        .field("lineResults", &PoemValidationResult::line_results)
        .field("meterResult", &PoemValidationResult::meter_result)
        .field("rhymeResult", &PoemValidationResult::rhyme_result)
        .field("summary", &PoemValidationResult::summary)
        .field("errorCount", &PoemValidationResult::error_count)
        .field("allErrors", &PoemValidationResult::all_errors);
    
    // Bind validator class
    class_<PoemValidator>("PoemValidator")
        .constructor<>()
        .function("addLine", &PoemValidator::add_line)
        .function("validateLineMeter", &PoemValidator::validate_line_meter)
        .function("setRhymeScheme", &PoemValidator::set_rhyme_scheme)
        .function("setMeter", &PoemValidator::set_meter)
        .function("validate", &PoemValidator::validate)
        .class_function("checkLine", &PoemValidator::check_line);
}
```

**JavaScript Usage**:
```javascript
const validator = new Module.PoemValidator();
validator.setRhymeScheme("ABAB");
validator.setMeter("x/x/x/x/x/");

// Add line and get immediate feedback
const lineResult = validator.addLine("Shall I compare thee to a summer's day?");

if (!lineResult.isValid) {
    console.error("Line has errors:");
    lineResult.unrecognizedWords.forEach(word => {
        console.log(`  Unknown word: ${word}`);
    });
} else {
    console.log(`Line has ${lineResult.possibleSyllableCounts[0]} syllables`);
}

// Add more lines
validator.addLine("Thou art more lovely and more temperate:");
validator.addLine("Rough winds do shake the darling buds of May:");
validator.addLine("And summer's lease hath all too short a date:");

// Validate entire poem
const result = validator.validate();

if (!result.isValid) {
    console.log("Validation errors:");
    console.log(result.summary);
    
    // Check specific line errors
    result.lineResults.forEach((lineResult, index) => {
        if (!lineResult.isValid) {
            console.log(`Line ${index}: ${lineResult.errorMessage}`);
        }
    });
    
    // Check meter errors
    if (!result.meterResult.isValid) {
        console.log("Meter errors:", result.meterResult.errorMessage);
    }
    
    // Check rhyme errors
    if (!result.rhymeResult.isValid) {
        console.log("Rhyme errors:", result.rhymeResult.errorMessage);
    }
}

// Or check a line without adding it
const testResult = Module.PoemValidator.checkLine("xyzzy abracadabra");
console.log("Unknown words:", testResult.unrecognizedWords);  // ["XYZZY", "ABRACADABRA"]
```

**TypeScript Definitions**:
```typescript
interface LineValidationResult {
    isValid: boolean;
    unrecognizedWords: string[];
    possibleSyllableCounts: number[];
    errorMessage: string;
    hasErrors(): boolean;
}

interface MeterValidationResult {
    isValid: boolean;
    matchesMeter: boolean;
    errorMessage: string;
    validStressPatterns: string[];
    expectedPattern: string;
}

interface RhymeValidationResult {
    isValid: boolean;
    nonRhymingPairs: [number, number][];
    rhymeDistances: number[];
    errorMessage: string;
}

interface PoemValidationResult {
    isValid: boolean;
    lineResults: LineValidationResult[];
    meterResult: MeterValidationResult;
    rhymeResult: RhymeValidationResult;
    summary: string;
    errorCount(): number;
    allErrors(): string[];
}

interface PoemValidator {
    addLine(text: string): LineValidationResult;
    validateLineMeter(lineIndex: number): MeterValidationResult;
    setRhymeScheme(pattern: string): void;
    setMeter(pattern: string): void;
    validate(): PoemValidationResult;
}

interface PoemValidatorStatic {
    new(): PoemValidator;
    checkLine(text: string): LineValidationResult;
}
```


#### 7. Performance Considerations

**WASM Performance Characteristics**:
- ✅ Near-native speed for CPU-bound operations (phonetic distance)
- ✅ Good for algorithmic work (our use case)
- ⚠️ Boundary crossing has overhead
- ⚠️ Memory allocation can be slower than native

**Optimization Strategy**:
```cpp
// Batch operations to minimize boundary crossings
class PoemValidator {
public:
    // ❌ BAD: Cross boundary for each line
    void validate_line(int index);
    
    // ✅ GOOD: Batch operation, single boundary crossing
    std::vector<LineResult> validate_all_lines();
    
    // ✅ GOOD: Stream results if needed
    void validate_lines_with_callback(
        emscripten::val callback  // JS function
    );
};
```

Streaming callback can be used in this way

```cpp
class PoemValidator {
public:
    // Stream validation results via callback
    void validate_lines_with_callback(emscripten::val js_callback) {
        for (size_t i = 0; i < m_lines.size(); ++i) {
            // Validate line
            auto result = validate_line_meter(i);
            
            // Call JS function with index and result
            js_callback(static_cast<int>(i), result);
            
            // Result is immediately passed to JS, then can be garbage collected
            // No need to accumulate all results in C++
        }
    }
};
```

```js
// Process results as they're computed
validator.validateLinesWithCallback((lineIndex, validationResult) => {
    // This function is called from C++ for each line
    console.log(`Line ${lineIndex}: ${validationResult.errorMessage}`);
    
    // Update UI immediately
    updateLineDisplay(lineIndex, validationResult);
});
```

#### 8. Threading Considerations

**Challenge**: WASM threading via Web Workers has limitations
**Implications**:
- Keep most operations single-threaded
- Thread-local storage may not work as expected
- Mutex/atomic still work but add overhead

**Decision**: 
- Design for single-threaded execution
- Make line-level caching thread-safe (future-proof)
- Allow parallel processing at poem level (multiple poems, not within)

```cpp
// Safe for single-threaded WASM, also thread-safe if needed later
class Line {
    mutable std::mutex m_cache_mutex;
    mutable std::optional<CachedData> m_cache;
    
    void ensure_cache() const {
        std::lock_guard lock(m_cache_mutex);
        if (!m_cache) {
            m_cache = compute_expensive_data();
        }
    }
};
```

### Revised Design Principles for WASM

1. ✅ **Domain-Driven Design**: Classes represent real poetry concepts
2. ✅ **Functional Programming**: Immutability, pure functions, composition
3. ✅ **Explicit over Implicit**: Make costs visible (loading, initialization)
4. ✅ **Simple Boundary**: Clean C++/JS interop via Embind
5. ✅ **Error Handling**: `std::expected` (no exceptions in public API)
6. ⚠️ **Lazy Evaluation**: Use carefully (adds complexity to boundary)
7. ✅ **Small Binary**: Minimize templates, avoid heavy stdlib

### Architecture Adjustments for WASM

#### Two-Layer API

**Layer 1: Core C++ API** (internal, full FP style)
- Rich types, ranges, functional composition
- Used for native C++ applications
- Full use of modern C++ features

**Layer 2: Web API** (public, WASM-friendly)
- Simple types, flat structure
- Optimized for JS interop
- Thin wrapper around core API

```cpp
// Layer 1: Core API (rich, functional)
namespace core {
    class Poem {
        // Full FP API with ranges, expected, etc.
        std::expected<ValidationResult, Error> validate() const;
        
        auto lines() const -> const std::vector<Line>&;
    };
}

// Layer 2: Web API (simple, flat)
namespace web {
    class PoemValidator {
        core::Poem m_poem;
        
    public:
        // Simple types for JS
        void add_line(std::string text);
        ApiResult validate();
        std::string to_json();
    };
}
```

#### Dictionary Loading Strategy

```cpp
class Dictionary {
    // Singleton for shared access
    static std::unique_ptr<Dictionary> s_instance;
    
public:
    // Explicit initialization from JS
    static std::expected<void, std::string> initialize(
        std::string_view cmudict_data
    );
    
    // Check if loaded
    static bool is_loaded();
    
    // For JS: estimate memory usage
    static size_t memory_usage();
    
    // Allow unloading if needed
    static void unload();
};
```

```javascript
// JS side: Control loading
class RhymeMeterLibrary {
    constructor() {
        this.module = null;
        this.dictLoaded = false;
    }
    
    async init() {
        // Load WASM module
        this.module = await createModule();
        
        // Check if dictionary already loaded (cached)
        if (this.module.Dictionary.isLoaded()) {
            this.dictLoaded = true;
            return;
        }
        
        // Load dictionary data
        const dictData = await this.fetchDictionary();
        const result = this.module.Dictionary.initialize(dictData);
        
        if (!result.success) {
            throw new Error(`Failed to load dictionary: ${result.error}`);
        }
        
        this.dictLoaded = true;
    }
    
    async fetchDictionary() {
        // Could be:
        // - Fetched from CDN
        // - Loaded from IndexedDB cache
        // - Lazy loaded chunks
        const response = await fetch('/data/cmudict.txt');
        return await response.text();
    }
}
```

---

## Design Principles
1. **Domain-Driven Design**: Classes represent real poetry concepts
2. **Encapsulation**: Objects own their data and operations
3. **Single Responsibility**: Each class has one clear purpose
4. **Functional Programming**: Emphasize immutability, pure functions, and composition
5. **Lazy Evaluation**: Compute expensive things only when needed
6. **Error Handling**: Consistent use of `std::expected` (monadic error handling)


### FP Patterns in the API

#### Monadic Chaining with std::expected
```cpp
// Chain operations that can fail
auto result = dictionary.lookup("fire")
    .and_then([](auto pronunciations) {
        return get_stress_patterns(pronunciations);
    })
    .transform([](auto patterns) {
        return patterns[0];  // Take first pattern
    })
    .or_else([](auto error) {
        return default_stress_pattern();
    });
```

#### Functional Pipelines for Line Processing

**How it works**: C++20 ranges allow you to compose operations using the pipe operator `|`, similar to Unix pipes or functional languages like F# and Elixir.

```cpp
// Transform collections functionally using ranges
auto syllable_counts = poem.lines()              // 1. Get vector<Line>
    | std::views::transform([](const Line& line) {  // 2. Map each Line to int
        return line.syllable_count(); 
    })
    | std::views::filter([](int count) {         // 3. Keep only counts > 5
        return count > 5; 
    })
    | std::ranges::to<std::vector>();            // 4. Materialize to vector
```


## Core Classes

### 1. Dictionary
**Responsibility**: Word-to-pronunciation mapping

**API**:
```cpp
class Dictionary {
private:
    std::unordered_map<std::string, std::vector<std::string>> m_dictionary;

public:
    // Static factory for shared dictionary instance
    static const Dictionary& instance();
    
    // Core lookup functionality
    std::expected<std::vector<std::string>, Error> lookup(const std::string& word) const;
    
    // Batch operations
    struct LookupResult {
        std::vector<std::pair<std::string, std::vector<std::string>>> found;
        std::vector<std::string> not_found;
    };
    LookupResult lookup_many(const std::vector<std::string>& words) const;
    
    // Utility methods
    bool contains(const std::string& word) const;
    size_t size() const;
};
```

**Notes**:
- Rename from `Phonetic` to `Dictionary` for clarity
- Singleton pattern for shared dictionary instance
- Keep focused on pure lookup functionality
- Move phonetic analysis methods elsewhere

---

### 2. Line
**Responsibility**: Represents a line of poetry with its phonetic properties

**API**:
```cpp
class Line {
private:
    std::string m_text;
    std::vector<std::string> m_words;
    
    // Lazily computed and cached (memoization pattern)
    mutable std::optional<std::vector<std::vector<std::string>>> m_pronunciation_combinations;
    mutable std::optional<std::vector<std::string>> m_failed_words;
    
    const Dictionary& m_dict;
    
    // Helper to compute pronunciations on demand (pure function, result cached)
    void ensure_pronunciations() const;

public:
    // Construction
    explicit Line(std::string text, const Dictionary& dict = Dictionary::instance());
    
    // Accessors
    const std::string& text() const { return m_text; }
    const std::vector<std::string>& words() const { return m_words; }
    
    // Phonetic operations (lazy evaluation + caching)
    std::expected<std::vector<std::vector<std::string>>, std::vector<std::string>> 
        pronunciation_combinations() const;
    
    // Syllable operations
    std::expected<std::vector<int>, std::vector<std::string>> 
        possible_syllable_counts() const;
    
    bool has_syllable_count(int count) const;
    
    // Rhyming operations
    std::expected<std::vector<std::string>, std::vector<std::string>> 
        rhyming_parts() const;
    
    int rhyme_distance_to(const Line& other) const;
    
    // Meter operations
    std::expected<std::vector<std::vector<int>>, std::vector<std::string>> 
        stress_patterns() const;
    
    bool fits_meter(const Meter& meter) const;
    
    // Validation
    bool is_valid() const;  // All words found in dictionary
    const std::vector<std::string>& failed_words() const;
    
    // Monadic operations for chaining
    std::expected<Line, std::vector<std::string>> validate() const;
    
    // Higher-order function support
    // Peter question: is this how we'll perform operations on Line from other Classes?\
    // ex: `auto syllable_counts = line.transform_phones(count_syllables);`
    template<typename Fn>
    auto transform_phones(Fn&& fn) const -> std::expected<
        std::vector<decltype(fn(std::declval<std::vector<std::string>>()))>,
        std::vector<std::string>
    > {
        return pronunciation_combinations()
            .transform([&](const auto& combos) {
                std::vector<decltype(fn(std::declval<std::vector<std::string>>()))> results;
                for (const auto& combo : combos) {
                    results.push_back(fn(combo));
                }
                return results;
            });
    }
};
```

**Key Features**:
- **Immutable**: constructed once, data doesn't change (except memoization cache)
- **Lazy evaluation**: pronunciations computed only when needed
- **Caching/Memoization**: expensive computations cached (functional optimization)
- **Pure methods**: all public methods are const and deterministic
- **Monadic interface**: operations return `std::expected` for chaining

---

### 3. Meter
**Responsibility**: Represents a meter pattern as a first-class object

**API**:
```cpp
class Meter {
private:
    std::set<std::vector<int>> m_valid_patterns;  // All patterns after expanding optionals
    std::string m_original_pattern;               // For display/debugging

public:
    // Construction via parsing
    static std::expected<Meter, MeterError> parse(const std::string& pattern);
    
    // Query operations
    bool accepts(const std::vector<int>& stress_pattern) const;
    bool accepts(const Line& line) const;
    
    int min_syllables() const;
    int max_syllables() const;
    
    const std::set<std::vector<int>>& valid_patterns() const { return m_valid_patterns; }
    const std::string& pattern() const { return m_original_pattern; }
    
    // Comparison
    bool operator==(const Meter& other) const;
};
```


```cpp
// ✅ Static factory - can return expected
class Meter {
private:
    // Private constructor - prevents direct construction
    Meter(std::set<std::vector<int>> patterns, std::string original)
        : m_valid_patterns(std::move(patterns))
        , m_original_pattern(std::move(original)) {}

public:
    // Named constructor - returns expected with error info
    static std::expected<Meter, MeterError> parse(const std::string& pattern) {
        // Try to parse
        auto patterns = parse_meter_pattern(pattern);
        
        if (!patterns) {
            // Return detailed error
            return std::unexpected(MeterError::UnclosedOptional);
        }
        
        // Success - construct and return
        return Meter{*patterns, pattern};
    }
};

// Usage - explicit error handling, no exceptions
auto result = Meter::parse("x/x/(");
if (!result) {
    std::cout << "Parse failed: " << result.error() << "\n";
} else {
    Meter meter = *result;
    // Use meter...
}
```

why: 5. Can Cache/Reuse Objects
```cpp
class Meter {
    static std::unordered_map<std::string, Meter> s_cache;
    
public:
    static std::expected<Meter, MeterError> parse(const std::string& pattern) {
        // Check cache first
        if (s_cache.contains(pattern)) {
            return s_cache[pattern];  // Return cached copy
        }
        
        // Parse and cache
        auto result = parse_impl(pattern);
        if (result) {
            s_cache[pattern] = *result;
        }
        return result;
    }
};
```

**Pattern Syntax**:
- `x` = unstressed syllable
- `/` = stressed syllable
- `(...)` = optional group
- Whitespace ignored
- Examples: `"x/x/"`, `"(x/)x/(x/)"`, `"x / x / x / x /"`

**Notes**:
- Immutable value object
- Parse once, check many times
- Pre-expands all optional combinations for fast checking

---

### 4. MeterScheme
**Responsibility**: Defines meter pattern for multiple lines

**API**:
```cpp
class MeterScheme {
private:
    std::vector<Meter> m_line_meters;

public:
    // Construction
    explicit MeterScheme(std::vector<Meter> meters);
    static std::expected<MeterScheme, MeterError> parse(const std::vector<std::string>& patterns);
    
    // Single meter for all lines
    static std::expected<MeterScheme, MeterError> uniform(const std::string& pattern, size_t num_lines);
    
    // Validation
    struct ValidationResult {
        bool is_valid;
        std::vector<size_t> invalid_line_indices;
        std::vector<std::string> error_messages;
    };
    
    ValidationResult check(const std::vector<Line>& lines) const;
    bool accepts(const std::vector<Line>& lines) const;  // Simple bool check
    
    // Accessors
    size_t size() const { return m_line_meters.size(); }
    const Meter& operator[](size_t index) const { return m_line_meters[index]; }
};
```

**Example Usage**:
```cpp
// Iambic pentameter for a sonnet
auto meter_scheme = MeterScheme::uniform("x/x/x/x/x/", 14);

// Custom meter per line
auto meters = MeterScheme::parse({
    "x/x/x/x/",      // Line 1: iambic tetrameter
    "x/x/x/x/x/",    // Line 2: iambic pentameter
    "x/x/x/",        // Line 3: iambic trimeter
});
```

---

### 5. RhymeScheme
**Responsibility**: Defines rhyme pattern for multiple lines

**API**:
```cpp
class RhymeScheme {
private:
    std::string m_pattern;  // e.g., "ABAB"
    std::unordered_map<char, std::vector<size_t>> m_rhyme_groups;

public:
    // Construction
    explicit RhymeScheme(std::string pattern);
    static std::expected<RhymeScheme, RhymeError> parse(const std::string& pattern);
    
    // Validation
    struct ValidationResult {
        bool is_valid;
        std::vector<std::pair<size_t, size_t>> failing_pairs;  // Line indices that should rhyme but don't
        std::unordered_map<char, std::vector<size_t>> rhyme_groups;
        std::vector<std::pair<size_t, size_t>> rhyme_distances;  // For each rhyming pair
    };
    
    ValidationResult check(const std::vector<Line>& lines, int max_distance = 0) const;
    bool accepts(const std::vector<Line>& lines, int max_distance = 0) const;
    
    // Accessors
    const std::string& pattern() const { return m_pattern; }
    size_t size() const { return m_pattern.size(); }
    const std::unordered_map<char, std::vector<size_t>>& rhyme_groups() const;
};
```

**Pattern Syntax**:
- Letters indicate rhyme groups: `"ABAB"`, `"AABB"`, `"ABCABC"`
- `X` or `-` indicates no rhyme required
- Case insensitive

**Example Usage**:
```cpp
auto shakespearean_sonnet = RhymeScheme::parse("ABABCDCDEFEFGG");
auto couplets = RhymeScheme::parse("AABBCCDD");
auto limerick = RhymeScheme::parse("AABBA");
```

---

### 6. Poem
**Responsibility**: Composes lines with optional rhyme and meter schemes

**API**:
```cpp
class Poem {
private:
    std::vector<Line> m_lines;
    std::optional<RhymeScheme> m_rhyme_scheme;
    std::optional<MeterScheme> m_meter_scheme;
    const Dictionary& m_dict;

public:
    // Construction
    explicit Poem(const Dictionary& dict = Dictionary::instance());
    
    // Builder pattern
    Poem& add_line(const std::string& text);
    Poem& add_lines(const std::vector<std::string>& texts);
    Poem& with_rhyme_scheme(const std::string& pattern);
    Poem& with_meter_scheme(const std::vector<std::string>& patterns);
    Poem& with_uniform_meter(const std::string& pattern);
    
    // Validation
    struct ValidationResult {
        bool is_valid;
        
        // Line-level issues
        std::vector<size_t> lines_with_unknown_words;
        std::unordered_map<size_t, std::vector<std::string>> unknown_words_by_line;
        
        // Meter issues
        std::optional<MeterScheme::ValidationResult> meter_validation;
        
        // Rhyme issues
        std::optional<RhymeScheme::ValidationResult> rhyme_validation;
        
        // Summary message
        std::string summary() const;
    };
    
    ValidationResult validate() const;
    bool is_valid() const;
    
    // Accessors
    const std::vector<Line>& lines() const { return m_lines; }
    const std::optional<RhymeScheme>& rhyme_scheme() const { return m_rhyme_scheme; }
    const std::optional<MeterScheme>& meter_scheme() const { return m_meter_scheme; }
    
    // Formatting
    std::string to_string() const;
    
    // Analysis
    std::vector<std::vector<int>> syllable_counts() const;
    std::vector<int> line_lengths() const;  // Character count
};
```

**Example Usage**:
```cpp
Poem sonnet;
sonnet.add_lines({
    "Shall I compare thee to a summer's day?",
    "Thou art more lovely and more temperate:",
    // ... 12 more lines
})
.with_rhyme_scheme("ABABCDCDEFEFGG")
.with_uniform_meter("x/x/x/x/x/");

auto result = sonnet.validate();
if (!result.is_valid) {
    std::cout << result.summary() << std::endl;
}
```

---

## Supporting Classes & Free Functions

### Error Types (Algebraic Data Types)
```cpp
enum class MeterError {
    NestedOptional,
    UnclosedOptional,
    UnrecognizedCharacter,
    EmptyPattern,
    InvalidLength
};

enum class RhymeError {
    EmptyPattern,
    InvalidCharacter,
    InvalidLength
};

enum class DictionaryError {
    WordNotFound,
    EmptyWord
};

// Helper for std::visit pattern matching
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
```

### Free Functions for Phonetic Operations

**Rationale**: Pure operations that don't need object state should be free functions.
This makes them more composable and testable.

```cpp
namespace phonetic {
    // Pure phonetic distance calculation
    int distance(
        const std::vector<std::string>& phones1,
        const std::vector<std::string>& phones2
    );
    
    // Partial application for use with algorithms
    auto distance_to(const std::vector<std::string>& target) {
        return [target](const std::vector<std::string>& phones) {
            return distance(phones, target);
        };
    }
    
    // Get rhyming part (pure function)
    std::vector<std::string> rhyming_part(const std::vector<std::string>& phones);
    
    // Stress pattern extraction (pure function)
    std::vector<int> stress_pattern(const std::vector<std::string>& phones);
    
    // Syllable counting (pure function)
    int syllable_count(const std::vector<std::string>& phones);
}

namespace meter {
    // Pure meter checking
    bool matches(
        const std::vector<int>& stress_pattern,
        const Meter& meter
    );
    
    // Parse meter string (pure, returns expected)
    std::expected<Meter, MeterError> parse(const std::string& pattern);
}

namespace rhyme {
    // Calculate rhyme distance between two lines
    std::expected<int, std::vector<std::string>> distance(
        const Line& line1,
        const Line& line2
    );
    
    // Check if two lines rhyme within threshold
    std::expected<bool, std::vector<std::string>> rhymes(
        const Line& line1,
        const Line& line2,
        int max_distance = 0
    );
    
    // Curried version for partial application
    auto rhymes_with(const Line& target, int max_distance = 0) {
        return [&target, max_distance](const Line& line) {
            return rhymes(line, target, max_distance);
        };
    }
}
```

### Vowel Distance Module

**Responsibility**: Calculate perceptual distance between vowels using hexagonal graph model

**Design Philosophy**: Pure free functions + immutable data singleton

**Rationale**: 
- Vowel distances are pre-calculated (15 vowels = 120 pairs = ~15 KB)
- Performance: 14.5x faster than BFS calculation (0.9μs vs 13.1μs per lookup)
- Memory cost is negligible (~15 KB) for the massive performance gain
- Fixed dataset (won't grow) - perfect candidate for pre-calculation
- Thread-safe singleton with lazy initialization (Meyers singleton pattern)

**The Vowel Hexagonal Graph Model:**

The vowel space is modeled as a hexagonal grid where adjacent vowels are perceptually similar:

```
         .          .         ..           
      .=.  ::.  .::  .=.   .=.  ::.        
    .=.       .::.       .=.       .:       
       IY    =    IH   .    UH    =    UW       
             =         .          =             
            .=.        ..        .=.            
         .-.   ::   .=    +.   -:   .-.        
       .-         -.        .-         -.       
          EH    :    AH    :    AO   -          
                :          :         -          
               .-.        .-.        -          
             ..  .-.  .-.  ..=   =..            
           .=.        ..         =              
               AE   .   AA    =                 
                    .          =                 
                  .::.      ..=                 
                .-.    .=. ::.                  
```

**API Design:**

```cpp
namespace vowel {
    /**
     * Immutable distance table - lazily initialized singleton
     * Pre-calculates all vowel-to-vowel distances on first access
     */
    class DistanceTable {
    private:
        struct PairHash {
            template <typename T1, typename T2>
            std::size_t operator()(const std::pair<T1, T2>& p) const {
                std::hash<T1> h1;
                std::hash<T2> h2;
                return h1(p.first) ^ (h2(p.second) << 1);
            }
        };
        
        struct PairEqual {
            template <typename T1, typename T2>
            bool operator()(const std::pair<T1, T2>& p1, const std::pair<T1, T2>& p2) const {
                return (p1.first == p2.first && p1.second == p2.second)
                    || (p1.first == p2.second && p1.second == p2.first);
            }
        };
        
        std::unordered_map<std::pair<std::string, std::string>, int, PairHash, PairEqual> m_distances;
        std::unordered_map<std::string, std::vector<std::string>> m_adjacency;
        
        // Private constructor
        DistanceTable(/*...*/);
        
    public:
        // Meyers singleton - thread-safe, lazy initialization
        static const DistanceTable& instance();
        
        // Pure lookup functions - const, no side effects
        int get_distance(std::string_view vowel1, std::string_view vowel2) const;
        const std::vector<std::string>& get_neighbors(std::string_view vowel) const;
        bool contains(std::string_view vowel) const;
        std::vector<std::string> all_vowels() const;
    };
    
    //
    // FREE FUNCTIONS - Pure operations on vowels
    //
    
    /**
     * Get distance between two vowels using hex graph model.
     * 
     * Pure function - thread-safe, no side effects.
     * Uses pre-calculated distances (0.9μs lookup time).
     * 
     * First call initializes singleton (~10ms one-time cost).
     * All subsequent calls are instant (just table lookup).
     * 
     * @param vowel1 ARPABET vowel (e.g., "AE", "IY")
     * @param vowel2 ARPABET vowel
     * @return distance in hex-graph steps (0 if same, -1 if not found)
     * 
     * Examples:
     *   distance("AE", "AE") = 0   // Same vowel
     *   distance("AE", "AH") = 1   // Adjacent
     *   distance("AE", "IY") = 3   // Three steps away
     */
    int distance(std::string_view vowel1, std::string_view vowel2);
    
    /**
     * Check if two vowels are adjacent on the hex graph.
     */
    bool are_adjacent(std::string_view vowel1, std::string_view vowel2);
    
    /**
     * Get all vowels adjacent to a given vowel.
     */
    std::vector<std::string> neighbors(std::string_view vowel);
    
    /**
     * Check if a string is a valid vowel in our model.
     * 
     * Valid vowels: AE, AA, EH, AH, AO, IY, IH, UH, UW, ER (monophthongs)
     *               AW, AY, EY, OW, OY (diphthongs)
     */
    bool is_valid_vowel(std::string_view vowel);
    
    /**
     * Partial application helper - create distance-to function.
     * 
     * Example:
     *   auto dist_to_AE = distance_to("AE");
     *   int d1 = dist_to_AE("IY");  // 3
     *   int d2 = dist_to_AE("AA");  // 1
     * 
     *   // Use with algorithms
     *   std::ranges::sort(vowels, {}, distance_to("AH"));
     */
    inline auto distance_to(std::string target) {
        return [target = std::move(target)](std::string_view vowel) {
            return distance(vowel, target);
        };
    }
    
    /**
     * For testing: recalculate distance using BFS.
     * Verifies pre-calculated table is correct.
     */
    int calculate_distance_bfs(
        std::string_view vowel1, 
        std::string_view vowel2,
        const DistanceTable& table = DistanceTable::instance()
    );
}
```

**Implementation Notes:**

```cpp
// Singleton initialization (happens once on first access)
const DistanceTable& DistanceTable::instance() {
    static const DistanceTable table = []() {
        // Runs ONCE - static local variable initialization
        // ~10ms to build adjacency map and calculate all distances
        auto adjacency = build_adjacency_map();      // Define edges
        auto distances = build_distance_map(adjacency);  // BFS for all pairs
        return DistanceTable{std::move(distances), std::move(adjacency)};
    }();
    return table;  // All subsequent calls return same instance
}

// Free function - delegates to singleton
int distance(std::string_view vowel1, std::string_view vowel2) {
    return DistanceTable::instance().get_distance(vowel1, vowel2);
}
```

**Integration with Phonetic Distance:**

```cpp
namespace phonetic {
    int distance(
        const std::vector<std::string>& phones1,
        const std::vector<std::string>& phones2
    ) {
        // Align phoneme sequences
        auto alignment = align(phones1, phones2);
        
        int total_cost = 0;
        for (const auto& [phone1, phone2] : alignment) {
            if (is_vowel(phone1)) {
                // Use vowel distance for vowels
                total_cost += vowel::distance(phone1, phone2) * VOWEL_COEFFICIENT;
            } else {
                // Use consonant distance for consonants
                total_cost += consonant::distance(phone1, phone2);
            }
        }
        return total_cost;
    }
}
```

**Performance Characteristics:**

```cpp
// Memory: ~15 KB (negligible)
// - 120 distance entries (15 vowels choose 2)
// - 15 adjacency lists
// - Hash map overhead

// Speed:
// - First call: ~10ms (initialization + lookup)
// - Subsequent calls: ~0.9μs (just lookup)
// - 14.5x faster than BFS calculation
// - Thread-safe (immutable after initialization)

// Startup cost in context:
// WASM module load: ~500ms total
//   - Download: ~400ms
//   - Compile: ~50-100ms
//   - Initialize: ~10ms
//     └─ Vowel table: ~10ms  ← 2% of total load time
```

**Usage Examples:**

```cpp
// Basic usage
int d = vowel::distance("AE", "IY");  // 3
bool adj = vowel::are_adjacent("AE", "AH");  // true

// Functional composition
auto dist_to_AH = vowel::distance_to("AH");
std::vector<std::string> vowels = {"IY", "IH", "EH", "AE", "AA"};
std::ranges::sort(vowels, {}, dist_to_AH);  // Sort by distance to AH

// Validation
if (!vowel::is_valid_vowel(input)) {
    return std::unexpected("Invalid vowel: " + input);
}

// Testing - verify pre-calculated values
for (const auto& v1 : vowel::DistanceTable::instance().all_vowels()) {
    for (const auto& v2 : vowel::DistanceTable::instance().all_vowels()) {
        assert(vowel::distance(v1, v2) == vowel::calculate_distance_bfs(v1, v2));
    }
}
```

**WASM Integration:**

```cpp
// Embind bindings - simple and efficient
EMSCRIPTEN_BINDINGS(vowel_distance) {
    using namespace emscripten;
    function("vowelDistance", &vowel::distance);
    function("vowelsAreAdjacent", &vowel::are_adjacent);
    function("isValidVowel", &vowel::is_valid_vowel);
}
```

```javascript
// JavaScript usage
const dist = Module.vowelDistance("AE", "IY");  // 3
const valid = Module.isValidVowel("XYZ");  // false
```

**Design Decision: Pre-calculation vs Lazy Calculation**

✅ **Pre-calculate** (chosen):
- 15 KB memory cost is negligible
- 14.5x faster (0.9μs vs 13.1μs)
- Simpler code (no caching, no locks)
- Consistent performance (no cold-start penalty)
- Fixed dataset (15 vowels won't change)

❌ Lazy calculation would require:
- Thread-safe caching mechanism
- Inconsistent timing (cold vs warm)
- More complex code
- Minimal memory savings (<15 KB)
- Not worth the complexity

### Functional Utilities

```cpp
namespace functional {
    // Compose two functions
    template<typename F, typename G>
    auto compose(F&& f, G&& g) {
        return [f = std::forward<F>(f), g = std::forward<G>(g)](auto&& x) {
            return f(g(std::forward<decltype(x)>(x)));
        };
    }
    
    // Pipe operator for function composition
    template<typename T, typename F>
    auto operator|(T&& value, F&& fn) {
        return fn(std::forward<T>(value));
    }
    
    // Map over expected values
    template<typename T, typename E, typename F>
    auto fmap(const std::expected<T, E>& exp, F&& fn) 
        -> std::expected<decltype(fn(std::declval<T>())), E> 
    {
        if (exp) {
            return fn(*exp);
        }
        return std::unexpected(exp.error());
    }
    
    // Flatten nested expected values
    template<typename T, typename E>
    auto flatten(const std::expected<std::expected<T, E>, E>& exp) 
        -> std::expected<T, E> 
    {
        if (exp) {
            return *exp;
        }
        return std::unexpected(exp.error());
    }
    
    // flatMap (bind) for expected
    template<typename T, typename E, typename F>
    auto flatMap(const std::expected<T, E>& exp, F&& fn) 
        -> decltype(fn(std::declval<T>())) 
    {
        if (exp) {
            return fn(*exp);
        }
        using ReturnType = decltype(fn(std::declval<T>()));
        return std::unexpected(exp.error());
    }
}
```

### Functional Validation Framework

```cpp
namespace validation {
    // Validator type: function that returns expected<void, Error>
    template<typename T, typename Error>
    using Validator = std::function<std::expected<void, Error>(const T&)>;
    
    // Combine validators with AND logic
    template<typename T, typename Error>
    Validator<T, Error> all(std::vector<Validator<T, Error>> validators) {
        return [validators = std::move(validators)](const T& value) 
            -> std::expected<void, Error> 
        {
            for (const auto& validator : validators) {
                if (auto result = validator(value); !result) {
                    return result;
                }
            }
            return {};
        };
    }
    
    // Combine validators with OR logic
    template<typename T, typename Error>
    Validator<T, Error> any(std::vector<Validator<T, Error>> validators) {
        return [validators = std::move(validators)](const T& value) 
            -> std::expected<void, Error> 
        {
            for (const auto& validator : validators) {
                if (auto result = validator(value); result) {
                    return result;
                }
            }
            return std::unexpected(Error{/* all failed */});
        };
    }
    
    // Create predicate-based validator
    template<typename T, typename Error, typename Predicate>
    Validator<T, Error> from_predicate(Predicate pred, Error error) {
        return [pred = std::move(pred), error = std::move(error)](const T& value) 
            -> std::expected<void, Error> 
        {
            return pred(value) ? std::expected<void, Error>{} 
                               : std::unexpected(error);
        };
    }
}
```

### Usage Examples of Functional API

```cpp
// Example 1: Functional pipeline with free functions
auto process_line = [](const std::string& text) {
    return Line{text}
        .pronunciation_combinations()
        .transform([](auto combos) {
            return combos 
                | std::views::transform(phonetic::rhyming_part)
                | std::ranges::to<std::vector>();
        });
};

// Example 2: Compose validators
using namespace validation;

auto validate_sonnet_line = all<Line, std::string>({
    from_predicate(
        [](const Line& l) { return l.has_syllable_count(10); },
        "Must have 10 syllables"
    ),
    from_predicate(
        [](const Line& l) { return l.is_valid(); },
        "Must contain only dictionary words"
    ),
    from_predicate(
        [](const Line& l) { return l.fits_meter(Meter::parse("x/x/x/x/x/").value()); },
        "Must be iambic pentameter"
    )
});

// Example 3: Partially applied functions for filtering
auto lines = poem.lines();
auto long_lines = lines 
    | std::views::filter([](const Line& l) { 
        return l.possible_syllable_counts()
            .transform([](auto counts) { 
                return *std::ranges::max_element(counts) > 10; 
            })
            .value_or(false);
    })
    | std::ranges::to<std::vector>();

// Example 4: Monadic chaining with flatMap
auto result = dictionary.lookup("fire")
    .and_then([](auto pronunciations) -> std::expected<std::vector<std::string>, DictionaryError> {
        if (pronunciations.empty()) {
            return std::unexpected(DictionaryError::WordNotFound);
        }
        return phonetic::rhyming_part(phones_string_to_vector(pronunciations[0]));
    })
    .transform([](auto rhyme_part) {
        return fmt::format("Rhyming part: {}", fmt::join(rhyme_part, " "));
    });

// Example 5: Function composition
auto analyze = functional::compose(
    [](const Line& line) { return line.stress_patterns(); },
    [](auto patterns) { return patterns.value_or(std::vector<std::vector<int>>{}); },
    [](auto patterns) { return patterns.size(); }
);

auto pattern_count = analyze(Line{"To be or not to be"});
```

---

## Migration Strategy

### Phase 1: Create New Classes Alongside Old
1. Implement `Dictionary`, `Line`, `Meter` classes
2. Keep existing `Rhyme_and_Meter` class
3. Update tests to cover new classes
4. No breaking changes yet

### Phase 2: Implement Schemes and Poem
1. Add `MeterScheme`, `RhymeScheme`, `Poem`
2. Add integration tests
3. Create migration guide in documentation

### Phase 3: Deprecate Old API
1. Mark old methods as `[[deprecated]]`
2. Implement old methods as thin wrappers over new API
3. Update examples and documentation

### Phase 4: Remove Old Code (v2.0)
1. Remove deprecated code
2. Breaking change - major version bump

---

## Design Decisions & Trade-offs

### Lazy Evaluation + Caching in Line (Memoization Pattern)
**Pro**: Expensive phonetic operations only computed when needed (functional optimization)
**Con**: Adds complexity, mutable state in const methods
**WASM Impact**: Good - avoids unnecessary computation, but adds binary size
**Decision**: Worth it - typical usage will call multiple methods on same Line. Caching is encapsulated and thread-safe.

### Immutable Value Objects (Meter, RhymeScheme, Line)
**Pro**: Thread-safe, easier to reason about, can be shared, enables referential transparency
**Con**: Can't modify after construction, may need to copy for changes
**WASM Impact**: Neutral - immutability doesn't affect binary size
**Decision**: Core FP principle - immutability leads to fewer bugs. Use builder pattern where mutation-like behavior is needed.

### Free Functions vs Member Functions
**Pro**: Free functions are more composable, testable, and functional in style
**Con**: May seem less OOP, scattered across namespaces
**WASM Impact**: Good - easier to export selectively to JS
**Decision**: Pure operations (phonetic distance, parsing) are free functions. Operations needing state (caching) are member functions.

### Dictionary Singleton
**Pro**: Avoid loading dictionary multiple times (expensive), purely functional from user perspective
**Con**: Global state, harder to test with custom dictionaries
**WASM Impact**: Critical - must be explicit about when loading happens (affects page load)
**Decision**: Singleton with explicit initialization. JS controls when dictionary loads.

### Error Handling with std::expected (Monadic Error Handling)
**Pro**: Forces error handling, self-documenting, composable with `.and_then()`, functional style
**Con**: Requires C++23 or backport library
**WASM Impact**: Excellent - no exceptions crossing boundary, predictable behavior
**Decision**: Essential for WASM. Use std::expected everywhere in public API.

### Higher-Order Functions vs Templates
**Pro**: `std::function` is easier to use, type-erased, can store in containers
**Con**: Runtime overhead, type erasure cost
**WASM Impact**: Bad - `std::function` increases binary size significantly
**Decision**: Use templates for most cases. Only use `std::function` for user callbacks in web API.

### Ranges vs Container Returns
**Pro**: Ranges are lazy, composable, functional style
**Con**: More complex types, C++20 required
**WASM Impact**: Complex - ranges work in WASM but add binary size
**Decision**: Use ranges internally, return containers across JS boundary. Provide range-based API for C++ users.

---

## Open Questions

1. **Thread Safety**: Should Line's caching be thread-safe? Mutex vs atomic?
   - **FP Consideration**: Pure functions should be thread-safe. Use `std::mutex` for cache.
   - **WASM Consideration**: Single-threaded by default, but mutex doesn't hurt. Future-proof.

2. **Dictionary Loading**: Lazy load on first access or require explicit initialization?
   - **FP Consideration**: Lazy loading is functional, but makes timing unpredictable.
   - **WASM Consideration**: Must be explicit - affects page load time. User controls when.
   - **Decision**: Explicit initialization required.

3. **Pronunciation Selection**: When multiple pronunciations exist, how to choose?
   - **FP Consideration**: Return all possibilities, let user fold/reduce with custom function.
   - **WASM Consideration**: Returning all options across boundary is expensive.
   - **Decision**: Core API returns all, Web API returns "best" (configurable strategy).

4. **Rhyme Distance Threshold**: What's a good default for "acceptable" rhyme?
   - **FP Consideration**: No default - require explicit threshold (no magic numbers).
   - **WASM Consideration**: JS users expect sensible defaults.
   - **Decision**: No default in core API, provide default (0) in web API with docs.

5. **Meter Strictness**: Should we have "strict" vs "loose" meter checking?
   - **FP Consideration**: Provide pure functions for different strictness levels, compose them.
   - **Decision**: Single implementation, configurable parameters.

6. **API Style**: Fluent builder vs traditional setters for Poem?
   - **FP Consideration**: Immutable builder (returns new builder) is most functional.
   - **WASM Consideration**: Immutable builder creates many temporary objects crossing boundary.
   - **Decision**: Mutable builder for web API, immutable for core API.

7. **Functional Purity**: How much caching before we violate referential transparency?
   - **Decision**: Caching is acceptable optimization as long as it's invisible to caller.

8. **Partial Application**: Should we provide curried versions of all multi-arg functions?
   - **Decision**: Provide curried versions for commonly partially-applied functions (rhymes_with, distance_to).

9. **Binary Size Budget**: What's acceptable WASM bundle size?
   - **Target**: < 500KB WASM + < 200KB JS glue code (gzipped)
   - **Dictionary**: Load separately, not embedded in WASM
   - **Monitor**: Use `wasm-opt` and check size regularly

10. **Browser Compatibility**: What browsers to support?
    - **Minimum**: Chrome 91+, Firefox 89+, Safari 15+ (WebAssembly support)
    - **Features needed**: WebAssembly, fetch API, ES6 modules
    - **Fallback**: Provide JS-only fallback? Or require WASM support?

---

## Implementation Guidelines

### Code Style for FP

```cpp
// ✅ GOOD: Pure function, immutable, const-correct
int phonetic_distance(
    const std::vector<std::string>& phones1,
    const std::vector<std::string>& phones2
) {
    // No mutations, no side effects
    return calculate_distance(phones1, phones2);
}

// ❌ BAD: Mutation, side effects
void phonetic_distance(
    std::vector<std::string>& phones1,  // Non-const reference
    std::vector<std::string>& phones2,
    int& result                          // Output parameter
) {
    phones1 = normalize(phones1);        // Mutating input!
    result = calculate_distance(phones1, phones2);
}

// ✅ GOOD: Monadic chaining
auto result = dictionary.lookup(word)
    .and_then([](auto phones) { return parse_stresses(phones); })
    .transform([](auto stresses) { return count_syllables(stresses); });

// ❌ BAD: Manual error handling, branching
auto phones = dictionary.lookup(word);
if (!phones) {
    return std::unexpected(phones.error());
}
auto stresses = parse_stresses(*phones);
if (!stresses) {
    return std::unexpected(stresses.error());
}
return count_syllables(*stresses);

// ✅ GOOD: Immutable builder
auto poem = PoemBuilder{}
    .with_line("Line 1")
    .with_line("Line 2")
    .build();

// ❌ BAD: Mutable builder
PoemBuilder builder;
builder.add_line("Line 1");      // Mutation
builder.add_line("Line 2");
auto poem = builder.build();

// ✅ GOOD: Higher-order function
auto validator = validation::from_predicate(
    [](const Line& l) { return l.is_valid(); },
    "Invalid line"
);

// ✅ GOOD: Partial application
auto rhymes_with_fire = rhyme::rhymes_with(Line{"fire"}, max_distance=5);
auto rhyming_lines = lines | std::views::filter(rhymes_with_fire);
```

### Testing FP Code

```cpp
// Property-based testing
TEST_CASE("phonetic_distance is commutative") {
    for (auto [phones1, phones2] : test_cases) {
        REQUIRE(phonetic::distance(phones1, phones2) == 
                phonetic::distance(phones2, phones1));
    }
}

TEST_CASE("Line is immutable") {
    Line line{"Hello world"};
    auto copy = line;
    
    // Any operation should not mutate original
    line.pronunciation_combinations();
    REQUIRE(line.text() == copy.text());
}

TEST_CASE("Validators compose correctly") {
    auto v1 = validation::from_predicate([](int x) { return x > 0; }, "positive");
    auto v2 = validation::from_predicate([](int x) { return x < 10; }, "small");
    auto composed = validation::all<int, std::string>({v1, v2});
    
    REQUIRE(composed(5).has_value());
    REQUIRE(!composed(-1).has_value());
    REQUIRE(!composed(15).has_value());
}
```

---

## Next Steps

## Next Steps

### Phase 0: WASM Setup & Validation (Week 0)
1. [ ] Set up Emscripten build system
2. [ ] Create minimal WASM example with Embind
3. [ ] Test C++20 features in Emscripten (ranges, concepts, expected)
4. [ ] Measure baseline binary size
5. [ ] Create JS wrapper skeleton
6. [ ] Test dictionary loading strategies

### Phase 1: Core FP Infrastructure (Week 1)
1. [ ] Set up functional utilities (compose, pipe, fmap, flatMap)
2. [ ] Implement error types as algebraic data types
3. [ ] Create helper for pattern matching (overloaded)
4. [ ] Write tests for functional utilities
5. [ ] **WASM**: Verify all utilities work in WASM build

### Phase 2: Pure Functions (Week 2)
1. [ ] Implement phonetic namespace with pure functions
2. [ ] Implement meter namespace with pure parsing
3. [ ] Implement rhyme namespace with pure distance functions
4. [ ] Write property-based tests
5. [ ] **WASM**: Create Embind exports for key functions
6. [ ] **WASM**: Measure binary size impact

### Phase 3: Immutable Value Objects (Week 3)
1. [ ] Implement `Dictionary` class (immutable after loading)
2. [ ] Implement `Meter` class (immutable value type)
3. [ ] Implement `Line` class (immutable with memoization)
4. [ ] Write tests verifying immutability
5. [ ] **WASM**: Create web-friendly dictionary loader
6. [ ] **WASM**: Test memory usage with large dictionary

### Phase 4: Composable Abstractions (Week 4)
1. [ ] Implement validation framework
2. [ ] Implement `MeterScheme` and `RhymeScheme`
3. [ ] Implement `Poem` with immutable builder
4. [ ] Write integration tests
5. [ ] **WASM**: Implement `PoemValidator` web API
6. [ ] **WASM**: Create JS wrapper with Promise-based loading

### Phase 5: Web API Layer (Week 5)
1. [ ] Design and implement complete Embind bindings
2. [ ] Create TypeScript definitions
3. [ ] Implement async initialization pattern
4. [ ] Add error handling across boundary
5. [ ] Optimize for binary size (check template instantiations)
6. [ ] Write end-to-end browser tests

### Phase 6: Optimization & Polish (Week 6)
1. [ ] Run `wasm-opt` with aggressive optimization
2. [ ] Profile hot paths, optimize if needed
3. [ ] Minimize boundary crossings
4. [ ] Add browser compatibility tests
5. [ ] Create example web applications
6. [ ] Write migration guide from v0

### Phase 7: Documentation (Week 7)
1. [ ] Document FP patterns used
2. [ ] Create functional programming guide
3. [ ] Write WASM integration guide
4. [ ] Document build process for WASM
5. [ ] Create API reference (C++ and JS)
6. [ ] Add examples for common use cases
7. [ ] Performance guide (binary size, loading strategies)
4. [ ] Update all examples to show FP style
