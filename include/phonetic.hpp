#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <expected>

class Phonetic {
private:
    // map of words with their CMU ARPABET_pronunciations, separated by spaces
    std::unordered_map<std::string, std::vector<std::string>> m_dictionary {};

// TODO mark functions as const that don't change state

public:
    // Error type for std::expected
    struct Error {
        std::string unidentified_word;
    };

    // Result type for text_to_phones
    struct TextToPhonesResult {
        std::vector<std::pair<std::string, std::vector<std::string>>> words_with_pronunciations;
        std::vector<std::string> failed_words;
        bool has_failures() const { return !failed_words.empty(); }
    };

    // Runs import_dictionary();
    Phonetic();
    
    // assumes filepath "../data/CMUdict/cmudict-0.7b"
    bool import_dictionary();

    /**
     * Get array of possible pronunciations from CMUdict.
     * 
     * @param (string) word: English word to look up.
     * @return a vector of strings, of the possible 
     pronunciations recorded in the dictionary, each 
     of which are a string of ARAPBET phones separated 
     by spaces, or an error consisting of the unindentified word.
    */
    std::expected<std::vector<std::string>, Error> word_to_phones(std::string word);

    /**
     * Get an array of possible pronunciations of each word from a text.
     * 
     * @param text (string): text to look up
     * @return TextToPhonesResult containing an arrary of pairs of words and their pronunciations (if found), along with an array of words that were not found. 
    */
    TextToPhonesResult text_to_phones(const std::string & text);

     /**
     * Takes a string of space-separated CMUdict phones, returns a string of the stresses.
     * 
     * 0 = no stress, 1 = primary stress, 2 = secondary stress.
     * 
     * @param (string) phones: space-separated CMUdict phones
     * @return (string): String of stresses
    */
    std::string phones_to_stresses(const std::string& phones);

    /**
     * Takes an English word and returns a vector of possible stresses.
     * 
     * @param (string) word: English word
     * @return std::expected containing either vector of stress patterns or error
    */
    std::expected<std::vector<std::string>, Error> word_to_stresses(const std::string& word);

    /**
     * Take a string of space-separated CMUdict phones, returns number of syllables.
     * 
     * @param (string) phones: space-separated CMUdict phones
     * @return (int): number of syllables
    */
    int phone_to_syllable_count(const std::string& phones);

    /**
     * Takes an English word, returns a vector of possible stresses.
     * 
     * (Most words have a single syllable count even across multiple pronunciations, but this catches the outliers, e.g. "fire".)
     * 
     * @param (string) word: English word
     * @return std::expected containing either vector of syllable counts or error
    */
    std::expected<std::vector<int>, Error> word_to_syllable_counts(const std::string& word);

    /**
     * Get the rhyming part from a string of space-separated phones.
     * 
     * "Rhyming part" consists of: the accented vowel closest, to the end of the word.
     * 
     * @param phones (string): string of space-separated phones
     * @return (string): a string of space-separated phones
    */
    std::string get_rhyming_part(const std::string& phones);

    // TODO implement search
        // search by phones
        // search by stress
};


