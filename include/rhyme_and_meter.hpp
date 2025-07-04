#pragma once

#include "phonetic.hpp"
#include "Hirschberg.hpp"
#include "convenience.hpp"
#include "distance.hpp"
#include <expected>
#include <functional>
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
     * Convert text to phones using the phonetic dictionary.
     * 
     * @param text (string): string of english words to convert
     * @return TextToPhonesResult containing words and their pronunciations
    */
    Phonetic::TextToPhonesResult text_to_phones(const std::string& text);
    
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
     * @param pair_of_possible_pronunciations (pairs of vec of strings)
     * @return the minimum weighted edit distance
    */
    int minimum_rhyme_distance(const std::pair<std::vector<std::string>, std::vector<std::string>>& pair_of_possible_pronunciations);

    /**
     * Uses Hirschberg algorithm to get both an alignment that results in a minimum pronunciation distance between two strings of english text.
     */
    Alignment_And_Distance minimum_alignmment(const std::pair<std::vector<std::string>, std::vector<std::string>>& pair_of_possible_pronunciations);
    
    /**
     * Helper function that takes a text string and returns all possible pronunciation combinations.
     * 
     * @param text (string): text string to process
     * @return std::expected containing either all pronunciation combinations, or an error if any words failed to be identified
    */
    std::expected<std::vector<std::vector<std::string>>, UnidentifiedWords> get_text_pronunciation_combinations(const std::string& text);
    
    /**
     * Generic function that takes two strings of text and applies a comparison function to all possible pronunciation combinations.
     * 
     * This method runs text_to_phones() on each text string, then generates all possible combinations
     * of word pronunciations from both texts and applies the provided comparison function to each combination.
     * 
     * @param text1 (string): first text string to compare
     * @param text2 (string): second text string to compare
     * @param comparison_func (function): function that takes two vectors of strings (phonemes) and returns a result
     * @param min_func (function): function that compares two results and returns true if first is less than second
     * @return std::expected containing either the minimum result from the comparison function, or an error if any words failed to be identified
    */
    template<typename ResultType>
    std::expected<ResultType, UnidentifiedWords> compare_text_pronunciations(
        const std::string& text1, 
        const std::string& text2,
        std::function<ResultType(const std::vector<std::string>&, const std::vector<std::string>&)> comparison_func,
        std::function<bool(const ResultType&, const ResultType&)> min_func
    ) {
        // Get pronunciation combinations for both texts
        auto combinations1_result = get_text_pronunciation_combinations(text1);
        auto combinations2_result = get_text_pronunciation_combinations(text2);
        
        // Check if either text has unidentified words and collect all of them
        std::vector<std::string> all_failed_words;
        if (!combinations1_result.has_value()) {
            all_failed_words.insert(all_failed_words.end(), 
                combinations1_result.error().words.begin(), 
                combinations1_result.error().words.end());
        }
        if (!combinations2_result.has_value()) {
            all_failed_words.insert(all_failed_words.end(), 
                combinations2_result.error().words.begin(), 
                combinations2_result.error().words.end());
        }
        
        // If there are any failed words, return them all together
        if (!all_failed_words.empty()) {
            return std::unexpected(UnidentifiedWords{all_failed_words});
        }
        
        const auto& combinations1 = combinations1_result.value();
        const auto& combinations2 = combinations2_result.value();
        
        // Apply comparison function to all combinations and find minimum
        ResultType minimum_result{};
        bool first_flag = true;
        
        for (const auto& combination1 : combinations1) {
            // Convert combination1 to a single pronunciation string
            std::string combined_pronunciation1;
            for (size_t i = 0; i < combination1.size(); ++i) {
                if (i > 0) {
                    combined_pronunciation1 += " ";
                }
                combined_pronunciation1 += combination1[i];
            }
            
            auto phones_vector1 = phones_string_to_vector(combined_pronunciation1);
            
            for (const auto& combination2 : combinations2) {
                // Convert combination2 to a single pronunciation string
                std::string combined_pronunciation2;
                for (size_t i = 0; i < combination2.size(); ++i) {
                    if (i > 0) {
                        combined_pronunciation2 += " ";
                    }
                    combined_pronunciation2 += combination2[i];
                }
                
                auto phones_vector2 = phones_string_to_vector(combined_pronunciation2);
                
                // Apply the comparison function
                ResultType result = comparison_func(phones_vector1, phones_vector2);
                
                if (first_flag) {
                    minimum_result = result;
                    first_flag = false;
                } else {
                    if (min_func(result, minimum_result)) {
                        minimum_result = result;
                    }
                }
            }
        }
        
        return minimum_result;
    }
    
    /**
     * Convenience function to get minimum alignment distance between two texts.
     * 
     * @param text1 (string): first text string to compare
     * @param text2 (string): second text string to compare
     * @return std::expected containing either the minimum alignment distance, or an error if any words failed to be identified
    */
    std::expected<int, UnidentifiedWords> minimum_text_distance(const std::string& text1, const std::string& text2);
    
    /**
     * Convenience function to get minimum alignment between two texts.
     * 
     * @param text1 (string): first text string to compare
     * @param text2 (string): second text string to compare
     * @return std::expected containing either the minimum alignment, or an error if any words failed to be identified
    */
    std::expected<Alignment_And_Distance, UnidentifiedWords> minimum_text_alignment(const std::string& text1, const std::string& text2);
    
    /**
     *
     * @param line1 (string): string of english words
     * @param line2 (string): string of english words
     * @return std::expected containing either the rhyme distance, or a RhymeError if the operation fails
    */
    std::expected<int, UnidentifiedWords> get_end_rhyme_distance(const std::string& line1, const std::string& line2);
};


