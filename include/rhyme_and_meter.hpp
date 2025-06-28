#pragma once

#include "phonetic.hpp"
#include <expected>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class MeterError {
    NestedOptional,
    UnclosedOptional,
    UnrecognizedCharacter
};

std::string getErrorMessage(MeterError error);

class Rhyme_and_Meter {
private:
    // CMUDict phonetic class
    Phonetic dict{};

// TODO mark functions as const that don't change state

public:

    std::expected<std::vector<std::string>, Phonetic::Error> word_to_phones(const std::string& word);
    
     /**
     * Convert meter in form of "x/x /x/(x /)" to a set of vector<int>, where 'x' is 0 and '/' is 1, where the set contains all the possible meters that could conform to the options.
     * 
     * Also removes any whitespace.
     * 
     * Returns a MeterError if meter is invalid, using std::expected.
     * 
     * @param meter (string): meter string containing 'x', '/' and possible white-space
     * @return Expected containing either a Set of Vector of Ints or a MeterError
    */
    std::expected<std::set<std::vector<int>>, MeterError> fuzzy_meter_to_binary_set(const std::string& meter);

    /**
     * Struct to return validity of meter, along with list of unrecognized words.
    */
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
     * TODO: Include a parameter of key/value pairs, consisting of the meter of unrecognized words, for more graceful error handling.
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

    // Error type for rhyming functions
    struct UnidentifiedWords {
        std::vector<std::string> words;
    };

    /**
     * Takes two lines and returns possible pronunciations of the comparable rhyming parts. Currently uses the shortest rhyming part.
     * 
     * TODO: take the rhyming part from the first, apply it to the second.
     * 
     * TODO: If we know the meter, we want to choose the pronunciation that matches that meter. 
     * 
     * 
     * E.g. We'd like to be able to check "poet" against "know it"
     * 
     * @param line1 (string): string of english words
     * @param line2 (string): string of english words
     * @return std::expected containing either a pair of vectors of rhyming parts, or an error containing the unidentified words
    */
    std::expected<std::pair<std::vector<std::string>, std::vector<std::string>>, UnidentifiedWords> compare_end_line_rhyming_parts(const std::string& line1, const std::string& line2);
    
    /**
     * Gives the minimum rhmying distance between a pair of vectors of possible pronunciations.
     * 
     * TODO: Account for length of rhmying part. This should probably return an average of the distance over the syllable length?
     *
     * TODO: Accept user definined key/value pairs of unknown words + known words that they rhyme with, for graceful error correction. 
     * 
     * @param rhyming_part_pairs (pairs of vec of strings)
     * @return the minimum weighted edit distance
    */
    int minimum_rhyme_distance(const std::pair<std::vector<std::string>, std::vector<std::string>>& rhyming_part_pairs);

    /**
     * Compares the end rhyme distance of two words/lines.
     * 0     => perfect rhyme!
     * 1-5   => pretty dang close!
     * 6-10  => close
     * 11-20 => further
     *
     * @param line1 (string): string of english words
     * @param line2 (string): string of english words
     * @return std::expected containing either the rhyme distance, or a RhymeError if the operation fails
    */
    std::expected<int, UnidentifiedWords> get_end_rhyme_distance(const std::string& line1, const std::string& line2);
};


