#include "CMU_Dict.h"
#include "convenience.h"
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

    auto vec1 {phones_string_to_vector(phones1)};
    auto vec2 {phones_string_to_vector(phones2)};
    auto vec3 {phones_string_to_vector(phones3)};
    auto vec4 {phones_string_to_vector(phones4)};

    auto H_Return1 = Hirschberg(vec1, vec2);
    auto lev_distance1 = levenshtein_distance(phones1, phones2);
    auto H_Return2 = Hirschberg(vec2, vec3);
    auto lev_distance2 = levenshtein_distance(phones2, phones3);
    auto H_Return3 = Hirschberg(vec1, vec3);
    auto lev_distance3 = levenshtein_distance(phones1, phones3);
    auto H_Return4 = Hirschberg(vec1, vec4);
    auto lev_distance4 = levenshtein_distance(phones1, phones4);

    print_pair(H_Return1.ZWpair);
    std::cout << "Hirsh Distance: " << H_Return1.distance << std::endl;
    std:: cout << "Lev Distance: " << lev_distance1 << std::endl;
    std::cout << std::endl;
    print_pair(H_Return2.ZWpair);
    std::cout << "Hirsh Distance: " << H_Return2.distance << std::endl;
    std:: cout << "Lev Distance: " << lev_distance2 << std::endl;
    std::cout << std::endl;
    print_pair(H_Return3.ZWpair);
    std::cout << "Hirsh Distance: " << H_Return3.distance << std::endl;
    std:: cout << "Lev Distance: " << lev_distance3 << std::endl;
    std::cout << std::endl;
    print_pair(H_Return4.ZWpair);
    std::cout << "Hirsh Distance: " << H_Return4.distance << std::endl;
    std:: cout << "Lev Distance: " << lev_distance4 << std::endl;
    std::cout << std::endl;

    // // Measure time for Hirsch
    // auto start1 = std::chrono::high_resolution_clock::now();
    // for (int i{}; i < 10000; ++i){
    //     auto vec1 {phones_string_to_vector(phones1)};
    //     auto vec2 {phones_string_to_vector(phones2)};
    //     auto hirschberg1 = Hirschberg(vec1, vec2);
    // }
    // auto end1 = std::chrono::high_resolution_clock::now();
    // auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    // std::cout << "Hirschberg took " << duration1 << " microseconds.\n";

    //  // Measure time for Lev
    // auto start2 = std::chrono::high_resolution_clock::now();
    // for (int i{}; i < 10000; ++i){
    //     int lev_distance1 = levenshtein_distance(phones1, phones2);
    // }
    // auto end2 = std::chrono::high_resolution_clock::now();
    // auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    // std::cout << "Levenshtein took " << duration2 << " microseconds.\n";

    
}

