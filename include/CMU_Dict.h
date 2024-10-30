#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


class CMU_Dict {
private:
    // map of words with their CMU ARPABET_pronunciations, separated by spaces
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

    /**
     * Get array of possible pronunciations from CMUDict.
     * 
     * Throws a std::exception if word not found.
     * 
     * @param (string) word: English word to look up.
     * @return Vector of strings, of the possible pronunciations recorded in the dictionary, each of which are a string of ARAPBET phones separated by spaces.
    */
    std::vector<std::string> find_phones(std::string word);

    /**
     * Get an array of possible pronuncitation of each word from a text.
     * 
     * If a word is not found, bool is marked false.
     * 
     * @param text (string): text to look up
     * @return An array of pairs, each pair has 1. array of possible pronunciations, 2. a bool flag indicating if word was found.
    */
    std::vector<std::pair<std::vector<std::string>, bool>> text_to_phones(const std::string & text);


    // Takes a string of phones separated by spaces, returns a string of the stresses. 0 no stress, 1 primary stress, 2 secondary stress.
    std::string phone_stresses(const std::string& phones);

    // Takes an English word and returns a vector of possible stresses. 
    std::vector<std::string> word_to_stresses(const std::string& word);

    // Take a string of phones separeated by spaces, returns a int number of syllables
    int phone_syllables(const std::string& phones);

    // Takes an English word, returns a vector of possible stresses.
    std::vector<int> word_to_syllables(const std::string& word);

    /**
     * Convert meter in "x/x/x/x/" form to a vector of bools, where 'x' is false and '/' is true;
     * 
     * Also removes any whitespace.
     * 
     * @param meter (string): meter string containing 'x', '/' and possible white-space
     * @return Vector of Bools. 
    */
    std::vector<bool> meter_to_binary(const std::string& meter);

    std::set<std::vector<int>> fuzzy_meter_to_set(const std::string& meter);



    // Takes a string of text, and a meter, specified with 'x' and '/' ('x' is unstressed, '/' is stressed), e.g. "x/x/x/x/", returns a bool value of whether that meter could be valid for that string.
    bool check_meter_validity(const std::string& text, const std::string& meter);
};
