#include "CMU_Dict.h"
#include "convenience.h"

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


bool CMU_Dict::import_dictionary() {
    const std::string file_path{"/Users/peterchinman/Documents/0B-Coding/00_projects/rhyme-and-meter/data/cmudict-0.7b"};
    std::ifstream cmudict{file_path};
    if (!cmudict.is_open()) {
        std::cerr << "Failed to open the dictionary." << '\n';
        return false;
    }
    std::string line;
    while (std::getline(cmudict, line)) {
        // ignore leading ;
        if(line.empty() || line[0] == ';') {
            continue;
        }

        std::istringstream iss(line);

        // extract word up to white space   
        std::string word;
        iss >> word;

        // strip variation "(n)", used in CMU DICT for multiple entries of same word
        // instead each pronunciation is added to the vector m_dictionary[word]
        if (word.back() == ')'){
            word.pop_back();
            word.pop_back();
            word.pop_back();
        }
       
        // pronunciation is ARPABET symbols, separated by spaces
        // vowels end with a number indicating stress, 0 no stress, 1 primary stress, 2 secondary stress
        std::string pronunciation;
        std::getline(iss, pronunciation);

        // trim white space
        ltrim(pronunciation);
        rtrim(pronunciation);

        m_dictionary[word].push_back(pronunciation);
    }

    cmudict.close();
    return true;
}

// throws a std::exception if word not found
std::vector<std::string> CMU_Dict::find_phones(std::string word) {
    // capitalize all queries
    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
    auto it = m_dictionary.find(word);
    if (it != m_dictionary.end()) {
        return it->second;
    }
    else {
        throw std::runtime_error(word + " not found in dictionary.");
    }
}

std::vector<std::pair<std::vector<std::string>, bool>> CMU_Dict::text_to_phones(const std::string & text) {
    std::vector<std::pair<std::vector<std::string>, bool>> results{};
    std::vector<std::string> words {strip_punctuation(text)};

    for (const auto & w : words) {
        try {
            std::vector<std::string> phones{find_phones(w)};
            results.emplace_back(phones, true);
        } catch (const std::exception &) {
            results.emplace_back(std::vector<std::string>{}, false);
        }
    }

    return results;
}


std::string CMU_Dict::phone_stresses(const std::string& phones) {
    std::string stresses{};
    for (const auto & c : phones){
        if (c == '0' || c == '1' || c == '2') {
            stresses.push_back(c);
        }
    }
    return stresses;
}

std::vector<std::string> CMU_Dict::word_to_stresses(const std::string& word) {
    std::vector<std::string> stresses{};

    std::vector<std::string> phones{find_phones(word)};
    for (const auto & p : phones) {
        stresses.emplace_back(phone_stresses(p));
    }
    return stresses;
}

int CMU_Dict::phone_syllables(const std::string& phones) {
    return static_cast<int>(phone_stresses(phones).length());
}

std::vector<int> CMU_Dict::word_to_syllables(const std::string& word) {
    std::vector<int> syllables;
    std::vector<std::string> phones{find_phones(word)};
    for(const auto & p : phones) {
        syllables.emplace_back(phone_syllables(p));
    }
    return syllables;
}

std::vector<bool> CMU_Dict::meter_to_binary(const std::string& meter){
    std::vector<bool> binary{};

    for(const auto& c : meter){
        if (std::isspace(static_cast<unsigned char>(c))) continue;
        if (c == 'x') binary.emplace_back(0);
        if (c == '/') binary.emplace_back(1);
    }

    return binary;
}

std::set<std::vector<int>> CMU_Dict::fuzzy_meter_to_set(const std::string& meter){
    std::set<std::vector<int>> meters_set;

    std::vector<int> main_path{};
    std::vector<std::pair<std::vector<int>, bool>> optional_paths{std::make_pair(std::vector<int>{}, true)};
    bool in_optional{false};
    int optional_index{0};

    for(const auto& c : meter){
        if (std::isspace(static_cast<unsigned char>(c))) continue;

        if (c == 'x'){
            if (in_optional){
                for (auto & path : optional_paths){
                    if (path.second == true) {
                        path.first.emplace_back(0);
                    }
                }
            }
            else{
                main_path.emplace_back(0);
                for (auto & path : optional_paths) {
                    path.first.emplace_back(0);
                }
            }
        }
        else if (c == '/'){
            if (in_optional){
                for (auto & path : optional_paths){
                    if (path.second == true) {
                        path.first.emplace_back(1);
                    }
                }
            }
            else{
                main_path.emplace_back(1);
                for (auto & path : optional_paths) {
                    path.first.emplace_back(1);
                }
            }
        }
        else if (c == '(') {
            // mark yes we're in optional path
            in_optional = true;
            // copy all the current optional paths, setting their bool flag to true
            // so that every optional path we come to, we take all routes
            // but only copy all paths if our initial optional path is not empty
            if ((optional_paths.size() == 1) && (optional_paths[0].first.empty())) {
                continue;
            }
            else {
                for(auto & path : optional_paths) {
                    optional_paths.emplace_back(path.first, true);
                }
            }
            
        }
        else if(c == ')') {
            in_optional = false;
            // set all optional paths bool flags to false
            for(auto & path : optional_paths) {
                path.second = false;
            }
        }
    }

    meters_set.insert(main_path);
    for(const auto & path : optional_paths) {
        meters_set.insert(path.first);
    }

    return meters_set;
}

bool CMU_Dict::check_meter_validity(const std::string& text, const std::string& meter) {
    std::vector<std::vector<std::string>> phones_from_text{};

    

    // step thru phones_from_text
    // for each variation of each word
        // if variation has same binary meter as previous, skip
            // so we're only actually running down forking paths of words with different stress patters
        // check current variation against meter until it fails, then try another variation
        // failure is a multi-syllabic word that doesn't meet meter requirements
    // if one passes, break and return true
    
}
