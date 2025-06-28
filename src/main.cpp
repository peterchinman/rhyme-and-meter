#include "rhyme_and_meter.hpp"
#include "convenience.hpp"
#include "vowel_hex_graph.hpp"
#include "Hirschberg.hpp"
#include <iostream>
#include <chrono>
#include <sstream>

int main() {
    Rhyme_and_Meter dict; 
    std::string word_1{"loud"};
    std::string word_2{"cloud"};
    auto maybe_phones_1{dict.word_to_phones(word_1)};
    auto maybe_phones_2{dict.word_to_phones(word_2)};

    if (!maybe_phones_1.has_value()) return 1;
    auto phones_1{maybe_phones_1.value()};
    if (!maybe_phones_2.has_value()) return 1;
    auto phones_2{maybe_phones_2.value()};

    for (auto phone : phones_1) {
        std::cout << phone << std::endl;
    }
    
    const auto alignment{NeedlemanWunsch(phones_1, phones_2)};

    print_pair(alignment.ZWpair);
    return 0;
}
