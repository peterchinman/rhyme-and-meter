#include "CMU_Dict.h"
#include "VowelHexGraph.h"
#include "Hirschberg.hpp"
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
// FUNCTION TO OUTPUT TO CHECK METER
int main() {
    CMU_Dict dict{};
    dict.import_dictionary();
    std::string phones1 {"B AA1 R K"};
    std::string phones2 {"B AA1 R K IH0 NG"};
    std::string phones3 {"K IH1 NG"};
    std::string phones4 {"L AA1 R K"};

    auto vec1 {dict.phones_string_to_vector(phones1)};
    auto vec2 {dict.phones_string_to_vector(phones2)};
    auto vec3 {dict.phones_string_to_vector(phones3)};
    auto vec4 {dict.phones_string_to_vector(phones4)};

    auto H_Return1 = Hirschberg(vec1, vec2);
    auto edit_distance1 = dict.levenshtein_distance(phones1, phones2);
    auto H_Return2 = Hirschberg(vec2, vec3);
    auto edit_distance2 = dict.levenshtein_distance(phones2, phones3);
    auto H_Return3 = Hirschberg(vec1, vec3);
    auto edit_distance3 = dict.levenshtein_distance(phones1, phones3);
    auto H_Return4 = Hirschberg(vec1, vec4);
    auto edit_distance4 = dict.levenshtein_distance(phones1, phones4);

    print_pair(H_Return1.ZWpair);
    std:: cout << "Edit Distance: " << edit_distance1 << std::endl;
    std::cout << std::endl;
    print_pair(H_Return2.ZWpair);
    std:: cout << "Edit Distance: " << edit_distance2 << std::endl;
    std::cout << std::endl;
    print_pair(H_Return3.ZWpair);
    std:: cout << "Edit Distance: " << edit_distance3 << std::endl;
    std::cout << std::endl;
    print_pair(H_Return4.ZWpair);
    std:: cout << "Edit Distance: " << edit_distance4 << std::endl;
    std::cout << std::endl;
    
}

