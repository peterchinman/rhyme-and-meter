#include "rhyme_and_meter.hpp"
#include "convenience.hpp"
#include "vowel_hex_graph.hpp"
#include "Hirschberg.hpp"
#include <iostream>
#include <chrono>
#include <sstream>

int main() {
    Rhyme_and_Meter dict; 
    
    std::string word_1, word_2;
    
    std::cout << "Enter word 1: ";
    std::cin >> word_1;
    
    std::cout << "Enter word 2: ";
    std::cin >> word_2;
    
    auto maybe_phones_1{dict.word_to_phones(word_1)};
    if (!maybe_phones_1.has_value()) {
        return std::cout << "Error: " << maybe_phones_1.error().unidentified_word << std::endl, 1;
    };
    auto phones_1{maybe_phones_1.value()};

    auto maybe_phones_2{dict.word_to_phones(word_2)};
    if (!maybe_phones_2.has_value()) {
        return std::cout << "Error: " << maybe_phones_2.error().unidentified_word << std::endl, 1;
    };
    auto phones_2{maybe_phones_2.value()};

    auto alignment{dict.minimum_alignmment({phones_1, phones_2})};
    print_pair(alignment.ZWpair);

    std::cout << "Distance: " << alignment.distance << std::endl;

    return 0;
}
