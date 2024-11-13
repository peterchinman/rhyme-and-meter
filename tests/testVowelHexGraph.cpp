#include <catch2/catch_test_macros.hpp>
#include "VowelHexGraph.h"
#include <string>
#include <vector>

struct VowelFixture {
    mutable VowelHexGraph vowel_hex_graph{};

    VowelFixture() {
        vowel_hex_graph.initialize();
    }
};

TEST_CASE_PERSISTENT_FIXTURE(VowelFixture, "VowelHexGraph tests") {

    SECTION("VowelHexGraph initialize and add_edge working correctly") {
        std::vector<std::string> vowel_connections{vowel_hex_graph.get_connected_vowels("AE")};
        std::vector<std::string> expected{"AA", "EH", "AH"};
        bool all_match{true};

        for (const auto& vowel : vowel_connections) {
            if(std::find(expected.begin(), expected.end(), vowel) == expected.end()) {
                all_match = false;
            }
        }
        REQUIRE(vowel_connections.size() == expected.size());
        REQUIRE(all_match);
    }

    SECTION("VowelHexGraph::calculate_shortest_distance"){
        REQUIRE(vowel_hex_graph.calculate_shortest_distance("AE", "AE") == 0);
        REQUIRE(vowel_hex_graph.calculate_shortest_distance("AE", "AH") == 1);
        REQUIRE(vowel_hex_graph.calculate_shortest_distance("UW", "IH") == 2);
        REQUIRE(vowel_hex_graph.calculate_shortest_distance("AO", "IY") == 3);
    }

    SECTION("VowelHexGraph::calculate_all_distances") {
        REQUIRE(vowel_hex_graph.get_distance("AE", "AE") == 0);
        REQUIRE(vowel_hex_graph.get_distance("AE", "AH") == 1);
        REQUIRE(vowel_hex_graph.get_distance("AH", "AE") == 1);
        REQUIRE(vowel_hex_graph.get_distance("AO", "IY") == 3);
        REQUIRE(vowel_hex_graph.get_distance("IY", "AO") == 3);
    }
}
