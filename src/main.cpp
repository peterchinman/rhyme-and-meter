#include "rhyme_and_meter.hpp"
#include <iostream>
#include <string>

int main() {
    Rhyme_and_Meter dict;
    Phonetic phonetic_dict;

    std::string phones1, phones2; 

    std::cout << "Enter first string of phonemes: ";
    std::getline(std::cin, phones1);
    
    std::cout << "Enter second string of phonemes: ";
    std::getline(std::cin, phones2);
    
    std::cout << "Distance: " << levenshtein_distance(phones1, phones2) << std::endl;
    
    // std::string line1, line2;
    
    // std::cout << "Enter first line of text: ";
    // std::getline(std::cin, line1);
    
    // std::cout << "Enter second line of text: ";
    // std::getline(std::cin, line2);
    
    // auto alignment_result = dict.minimum_text_alignment(line1, line2);
    
    // if (!alignment_result.has_value()) {
    //     std::cout << "Error: Unrecognized words: ";
    //     const auto& error = alignment_result.error();
    //     for (size_t i = 0; i < error.words.size(); ++i) {
    //         if (i > 0) std::cout << ", ";
    //         std::cout << error.words[i];
    //     }
    //     std::cout << std::endl;
    //     return 1;
    // }
    
    // const auto& alignment = alignment_result.value();
    
    // std::cout << "Distance: " << alignment.distance << std::endl;

    // std::cout << "Distance per syllable: " << alignment.distance / phonetic_dict.phone_to_syllable_count(line1) << std::endl;
    
    // std::cout << "Alignment:" << std::endl;
    // print_pair(alignment.ZWpair);
    
    return 0;
}
