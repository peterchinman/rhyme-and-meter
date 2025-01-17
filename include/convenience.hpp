#pragma once

#include <iostream>
#include <string>
#include <sstream>

/**
 * Convert CMU style space-separated string of phones to vector of separate symbols.
 * 
 * @param phones (string): string of space-separated phones
 * @return (vector<string>): vector of phone symbols
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



