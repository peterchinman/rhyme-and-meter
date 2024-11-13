#include "CMU_Dict.h"
#include "VowelHexGraph.h"
#include <iostream>
#include <chrono>


// FUNCTION TO OUTPUT TO CHECK METER
// int main(int argc, char* argv[]) {
//     if (argc != 3) {
//         std::cerr << "Usage: " << argv[0] << " <text> <meter>" << std::endl;
//         return 1;
//     }
//     CMU_Dict dict;
//     bool dictBool = dict.import_dictionary();

//     std::string text = argv[1];
//     std::string meter = argv[2];

//     CMU_Dict::Check_Validity_Result result = dict.check_meter_validity(text, meter);

//     if (result.is_valid){
//         std::cout << "valid" << std::endl;
//         return 0;
//     }
//     else {
//         std::cout << "invalid" << std::endl;
//         return 1;
//     }
// }

int main() {

    auto start = std::chrono::high_resolution_clock::now();

    VowelHexGraph vowel_hex_graph;
    vowel_hex_graph.initialize();

    for (int i{}; i < 10000; ++i) {
        vowel_hex_graph.calculate_shortest_distance("IY", "AA");
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    auto start2 = std::chrono::high_resolution_clock::now();

    VowelHexGraph vowel_hex_graph2;
    vowel_hex_graph2.initialize();
    vowel_hex_graph2.calculate_all_distances();

    for (int i{}; i < 10000; ++i) {
        vowel_hex_graph2.get_distance("IY", "AA");
    }

    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

    std::cout << "Calculating live, time taken: " << duration.count() << " milliseconds" << std::endl;
    std::cout << "Calculating first, time taken: " << duration2.count() << " milliseconds" << std::endl;
}
