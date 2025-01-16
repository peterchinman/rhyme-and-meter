#include <catch2/catch_test_macros.hpp>
#include "vowel_hex_graph.hpp"
#include <string>
#include <vector>

struct VowelFixture {
    VowelFixture() {
        VowelHexGraph::initialize();
    }
};

TEST_CASE_PERSISTENT_FIXTURE(VowelFixture, "VowelHexGraph tests") {

    SECTION("VowelHexGraph initialize and add_edge working correctly") {
        std::vector<std::string> vowel_connections{VowelHexGraph::get_connected_vowels("AE")};
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
        REQUIRE(VowelHexGraph::calculate_shortest_distance("AE", "AE") == 0);
        REQUIRE(VowelHexGraph::calculate_shortest_distance("AE", "AH") == 1);
        REQUIRE(VowelHexGraph::calculate_shortest_distance("UW", "IH") == 2);
        REQUIRE(VowelHexGraph::calculate_shortest_distance("AO", "IY") == 3);
    }

    SECTION("VowelHexGraph::calculate_all_distances") {
        REQUIRE(VowelHexGraph::get_distance("AE", "AE") == 0);
        REQUIRE(VowelHexGraph::get_distance("AE", "AH") == 1);
        REQUIRE(VowelHexGraph::get_distance("AH", "AE") == 1);
        REQUIRE(VowelHexGraph::get_distance("AO", "IY") == 3);
        REQUIRE(VowelHexGraph::get_distance("IY", "AO") == 3);
    }

    SECTION("VowelHexGraph::calculate_all_distances but including diphthongs") {
        // THESE are all my opinionated diphthong adjacencies
        REQUIRE(VowelHexGraph::get_distance("AW", "UH") == 1);
        REQUIRE((VowelHexGraph::get_distance("AW", "OW") == 1));
        REQUIRE((VowelHexGraph::get_distance("AW", "AH") == 1));

        REQUIRE(VowelHexGraph::get_distance("AY", "IH") == 1);
        REQUIRE((VowelHexGraph::get_distance("AY", "EY") == 1));
        REQUIRE((VowelHexGraph::get_distance("AY", "AH") == 1));

        REQUIRE(VowelHexGraph::get_distance("EY", "IH") == 1);
        REQUIRE(VowelHexGraph::get_distance("EY", "EH") == 1);
        REQUIRE(VowelHexGraph::get_distance("EY", "IY") == 1);

        REQUIRE(VowelHexGraph::get_distance("OW", "OY") == 1);
        REQUIRE(VowelHexGraph::get_distance("OW", "UH") == 1);
        REQUIRE(VowelHexGraph::get_distance("OW", "UW") == 1);
        REQUIRE(VowelHexGraph::get_distance("OW", "AO") == 1);

        REQUIRE(VowelHexGraph::get_distance("OY", "IH") == 1);

    }
}
