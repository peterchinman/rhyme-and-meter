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

// TODO mark functions as const that don't change state

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
    std::vector<std::string> word_to_phones(std::string word);

    /**
     * Get an array of possible pronuncitation of each word from a text.
     * 
     * If a word is not found, vector will contain the word that was searched for, and bool is marked false.
     * 
     * @param text (string): text to look up
     * @return An array of pairs, each pair has 1. array of possible pronunciations, 2. a bool flag indicating if word was found.
    */
    std::vector<std::pair<std::vector<std::string>, bool>> text_to_phones(const std::string & text);


    // Takes a string of phones separated by spaces, returns a string of the stresses. 0 no stress, 1 primary stress, 2 secondary stress.
    std::string phone_to_stress(const std::string& phones);

    // Takes an English word and returns a vector of possible stresses. 
    std::vector<std::string> word_to_stresses(const std::string& word);

    // Take a string of phones separeated by spaces, returns a int number of syllables
    int phone_to_syllable_count(const std::string& phones);

    // Takes an English word, returns a vector of possible stresses.
    std::vector<int> word_to_syllables(const std::string& word);

     /**
     * Convert meter in form of "x/x /x/(x /)" to a set of vector<int>, where 'x' is 0 and '/' is 1, where the set contains all the possible meters that could conform to the options.
     * 
     * Also removes any whitespace.
     * 
     * @param meter (string): meter string containing 'x', '/' and possible white-space
     * @return Set of Vector of Ints.
    */
    std::set<std::vector<int>> fuzzy_meter_to_binary_set(const std::string& meter);


    struct Check_Validity_Result {
        bool is_valid{};
        std::vector<std::string> unrecognized_words;
    };

    /**
     * Check whether a given text and meter combination is valid.
     * 
     * Checks all possible pronunciations of each word in a text against all possible meters given optional meters.
     * 
     * Single-syllable words automatically passed, given the ambiguity of determining their meter. Multi-syllabic words are checked to see whether their pattern of stresses matches the meter, with various exceptions coded in. E.g. a "secondarily stressed" syllable can function within a meter as an ustressed syllable. (TODO: consider the scenario where an unstressed syllable can function within a meter as stressed...)
     * 
     * 
     * @param text (string): string of english words to check against the meter
     * @param meter (string): meter string containing 'x', '/' and possible white-space
     * @return a struct containing a bool is_valid and a vector of strings of unrecognized words
    */
    Check_Validity_Result check_meter_validity(const std::string& text, const std::string& meter);


    /**
     * Check whether a given text and syllable count combination is valid.
     * 
     * Checks all possible pronunciations of each word in a text against the syllable count
     * 
     * @param text (string): string of english words
     * @param syllable_cout (int): number of syllables
     * @return a struct containing a bool is_valid and a vector of strings of unrecognized words
    */
    Check_Validity_Result check_syllable_validity(const std::string& text, int syllable_count);

    /**
     * Get the rhyming part from a string of space-separated phones.
     * 
     * "Rhyming part" refers to the accented vowel closest to the end of the word, to the end of the word.
     * 
     * @param phones (string): string of space-separated phones
     * @return (string): a string of space-separated phones
    */
    std::string get_rhyming_part(const std::string& phones);

    /**
     * Convert CMU style space-separated string of phones to vector of separate symbols.
     * 
     * @param phones (string): string of space-separated phones
     * @return (vector<string>): vector of phone symbols
    */
    std::vector<std::string> phones_string_to_vector(const std::string& phones);

    /**
     * Implementation of levenshtein distance algorithm, but comparing ARPABET symbols, instead of characters, with anonymized vowels.
     * 
     * I.e. Comparing "K IH1 T AH0 N" and "S IH1 T IH0 NG", we will in fact compare {"K", vowel, "T", vowel, "N"} and {"S", vowel, "T", vowel, "NG"}, which gives us a distance of 2.
     * 
     * TODO: this does not take consonant distance into account. E.g. /B/ and /P/ should be closer than /B/ and /SH/. This is sort of a hack to get something meaningful in the meantime. But ulitmately we should incorporate consonant distance.
     *
     * 
     * @param phones1 (string): string of space-separated phones
     * @param phones2 (stinrg): string of space-separated phones
     * @return (int): levenshtein distance between the sets of phones
    */
    int levenshtein_distance(const std::string& phones1, const std::string& phones2);

    /**
     * Get the rhyming distance between two strings of phones. First, extracts the rhyming part of each, and then compares them.
     * 
     * 
     * 
     * If rhyming parts are identical distance = 0. 
     * If rhyming parts have identical consonant structure, distance =  sum of VowelHexGraph::get_distance(vowel1, vowel2) for each vowel in rhyming part
     * TODO: Is sum correct here? Or should they be, multiplied??
     * 
     * Mis-matched vowel stress = +1 distance.
     * 
     * 
     * @param phone1 (phones): string of space-separated phones
     * @param phone 2(string): string of space-separated phones
     * @return (string): a string of space-separated phones
    */
    int rhyme_distance(const std::string& phones1, const std::string& phones2);


};
