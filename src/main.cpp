#include "rhyme_and_meter.hpp"
#include "convenience.hpp"
#include "vowel_hex_graph.hpp"
#include "hirschberg.hpp"
#include <iostream>
#include <chrono>
#include <sstream>

void print_pair(std::pair< std::vector<std::string>, std::vector<std::string> > ZWpair ) {
    for (const auto & symbol : ZWpair.first) {
        std::cout << symbol;
        for(std::size_t i{}; i < 4 - symbol.size(); ++i){
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    for (const auto & symbol : ZWpair.second) {
        std::cout << symbol;
        for(std::size_t i{}; i < 4 - symbol.size(); ++i){
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

int main() {
    Rhyme_and_Meter dict{};

    return 0;
}

