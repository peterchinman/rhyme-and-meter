#include "CMU_Dict.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


bool CMU_Dict::import_dictionary() {
    const std::string file_path{"../../data/cmudict-0.7b"};
    std::ifstream cmudict{file_path};
    if (!cmudict.is_open()) {
        std::cerr << "Failed to open the dictionary." << '\n';
        return false;
    }
    std::string line;
    while (std::getline(cmudict, line)) {
        if(line.empty() || line[0] == ';') {
            continue;
        }

        std::istringstream iss(line);

        // extract word up to white space   
        std::string word;
        iss >> word;

        // strip variation "(n)", used in CMU DICT for multiple entries of same word
        // instead each pronunciation is added to the vector m_dictionary[word]
        std::size_t pos = word.find('(');
        if (pos != std::string::npos && pos!= 0) {
            word = word.substr(0, pos);
        }

        // extract pronunciation
        // pronunciation is ARPABET symbols, separated by spaces
        // TODO: consider whether this should instead be an array
        // vowels end with a number indicating stress, 0 no stress, 1 primary stress, 2 secondary stress
        std::string pronunciation;
        std::getline(iss, pronunciation);

        // remove two leading spaces
        pronunciation = pronunciation.substr(2);

        m_dictionary[word].push_back(pronunciation);
    }

    cmudict.close();
    return true;
}

// throws a std::exception if query not found
std::vector<std::string> CMU_Dict::search_dictionary(std::string query) {
    // capitalize all queries
    std::transform(query.begin(), query.end(), query.begin(), ::toupper);
    auto it = m_dictionary.find(query);
    if (it != m_dictionary.end()) {
        return it->second;
    }
    else {
        throw std::runtime_error(query + " not found in dictionary.");
    }
}

// vowels in cmudict end with a number as an accent indicator
// we *could* just use that in this function as an indicator that a symbol is a vowel
// instead here we strip the digit and then check against a list of vowels
// is that helpfully robust, or just an extra step? you tell me
int CMU_Dict::countVowels(const std::string& pronunciation) {
    std::istringstream iss(pronunciation);
    std::string symbol;
    int vowel_count{};

    while (iss >> symbol) {
        
        if (!symbol.empty() && std::isdigit(symbol.back())) {
            symbol.pop_back();
        }

        if(CMU_VOWELS.find(symbol) != CMU_VOWELS.end()) {
            ++vowel_count;
        }
    }

    return vowel_count;
}
