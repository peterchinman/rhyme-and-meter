#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


class CMU_Dict {
private:
    // map of words with CMU ARPABET_pronunciations
    std::unordered_map<std::string, std::vector<std::string>> m_dictionary {};

    // All vowels used in CMU
    const std::unordered_set<std::string> CMU_VOWELS = {
        "AA", "AE", "AH", "AO", "EH",
        "ER", "IH", "IY", "UH", "UW", // 10 monopthongs
        "AW", "AY", "EY", "OW", "OY" // 5 dipthongs
    };


public:
    // assumes filepath "../data/cmudict-0.7b"
    bool import_dictionary();

    // throws a std::exception if query not found
    std::vector<std::string> search_dictionary(std::string query);

    int countVowels(const std::string& pronunciation);
};
