#pragma once

#include <iostream>
#include <string>
#include <sstream>

/**
 * Convert CMU style space-separated string of phonemes to vector of separated symbols.
 * 
 * @param phones (string): string of space-separated phonemes
 * @return (vector<string>): vector of phoneme symbols
*/
inline std::vector<std::string> phones_string_to_vector(const std::string& phones) {
    std::vector<std::string> result{};
    std::istringstream iss{phones};
    std::string word{};
    while(iss >> word) {
        result.emplace_back(word);
    }
    return result;
}


/**
 * Convert vectors of separated phoneme symbols to CMU style space-separated string of phonemes.
 * 
 * @param phones (vector<string>): vector of phoneme symbols
 * @return (string): space-separated string of phonemes
*/
inline std::string phones_vector_to_string(std::vector<std::string> vector) {
    std::string result{};
    for (const auto& phone : vector) {
        if (!result.empty()) {
            result += " ";
        }
        result += phone;
    }
    return result;
}

// Naive hack relying on CMU phoneme format to detect vowels using the accent indicator.
inline bool is_vowel(const std::string& phoneme) {
    return std::isdigit(phoneme.back());
}   



