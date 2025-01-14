#include <catch2/catch_test_macros.hpp>
#include "distance.hpp"
#include "rhyme_and_meter.hpp"
#include "vowelhexgraph.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

struct Fixture {
    mutable Rhyme_and_Meter dict;
    mutable VowelHexGraph vowel_hex_graph{};
    // bool import_success{};

    // Fixture() {
        // bool import_success = dict.import_dictionary();
        // vowel_hex_graph.initialize();
    // }
};


TEST_CASE_PERSISTENT_FIXTURE(Fixture, "Dictionary tests") {
   
    SECTION("fuzzy_meter_to_binary_set") {
        std::string meter{"x/x/x/x/(x/)"};
        std::set<std::vector<int>> meters_set = dict.fuzzy_meter_to_binary_set(meter);
        REQUIRE(meters_set.size() == 2);
        std::vector<int> check1 = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check1) != meters_set.end());
        std::vector<int> check2 = {0, 1, 0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check2) != meters_set.end());
    }

    SECTION("fuzzy_meter_to_binary_set extra fuzzy edition") {
        std::string meter{"(x/)x/(x/)"};
        std::set<std::vector<int>> meters_set = dict.fuzzy_meter_to_binary_set(meter);

        // 3 possibilities here because 2 of the possibilites are the same and set doesn't store duplicates
        REQUIRE(meters_set.size() == 3);
        std::vector<int> check1 = {0, 1};
        REQUIRE(meters_set.find(check1) != meters_set.end());
        std::vector<int> check2 = {0, 1, 0, 1};
        REQUIRE(meters_set.find(check2) != meters_set.end());
        std::vector<int> check3 = {0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check3) != meters_set.end());

        // extra convoluted example
        std::string meter2 = "(/x)/x (/)x/(x)";
        std::set<std::vector<int>> meters_set2 = dict.fuzzy_meter_to_binary_set(meter2);
        std::vector<int> check4 = {1, 0, 1, 0, 1, 0, 1, 0};
        REQUIRE(meters_set2.size() == 8);
        REQUIRE(meters_set2.find(check4) != meters_set2.end());
    }

    SECTION("fuzzy_meter_to_binary_set exception handling") {
        // no closing parentheses
        std::string meter{"x/ ( x/"};
        std::set<std::vector<int>> meters_set{};
        REQUIRE_THROWS( meters_set = dict.fuzzy_meter_to_binary_set(meter));
        // no opening paren
        meter = "x/) /x/x";
        REQUIRE_THROWS( meters_set = dict.fuzzy_meter_to_binary_set(meter));
        // paren inside paren
        meter = "x/ ((xx)) /x";
        REQUIRE_THROWS( meters_set = dict.fuzzy_meter_to_binary_set(meter));
    }

    SECTION("check_meter_validity") {
        // single syllable test
        std::string text = "I want to suck your blood right now";
        std::string meter =         "x/x/x/x/";
        std::string short_meter =   "x/x/x/x";
        std::string long_meter =    "x/x/x/x/x";
        REQUIRE(dict.check_meter_validity(text, meter).is_valid  == true);
        REQUIRE(dict.check_meter_validity(text, short_meter).is_valid  == false);
        REQUIRE(dict.check_meter_validity(text, long_meter).is_valid  == false);
        // multi syllable test
        // KARAOKE  K EH2 R IY0 OW1 K IY0
        // OKEY-DOKEY  OW1 K IY0 D OW1 K IY0
                    text = "karaoke okey-dokey";
        std::string good_meter = "/x/x /x/x";
        std::string bad_meter = "x/x/ x/x/";
                    short_meter = "/x/x /x/";
        REQUIRE(dict.check_meter_validity(text, good_meter).is_valid  == true);
        REQUIRE(dict.check_meter_validity(text, bad_meter).is_valid  == false);
        REQUIRE(dict.check_meter_validity(text, short_meter).is_valid  == false);
    }

    SECTION("check_meter_validity optional meter") {
        // KARAOKE  K EH2 R IY0 OW1 K IY0
        // OKEY-DOKEY  OW1 K IY0 D OW1 K IY0
        std::string text = "karaoke okey-dokey";
        std::string good_meter = "/x/x (/)x/x";
        std::string good_meter2 = "(/x)/x (/)x/(x)";
        std::string bad_meter = "x/x(/ x)/x/";
        REQUIRE(dict.check_meter_validity(text, good_meter).is_valid  == true);
        REQUIRE(dict.check_meter_validity(text, good_meter2).is_valid  == true);
        REQUIRE(dict.check_meter_validity(text, bad_meter).is_valid  == false);
    }

    SECTION("check_meter_validity complicated example") {
        // KARAOKE  K EH2 R IY0 OW1 K IY0
        // OKEY-DOKEY  OW1 K IY0 D OW1 K IY0
        std::string text = "fire conflicts content record";
        std::string good_meter = "/ /x /x /x";
        std::string good_meter2 = "/x x/ /x x/";
        std::string bad_meter = "/(x) x/ x/ xx";
        REQUIRE(dict.check_meter_validity(text, good_meter).is_valid == true);
        REQUIRE(dict.check_meter_validity(text, good_meter2).is_valid == true);
        REQUIRE(dict.check_meter_validity(text, bad_meter).is_valid == false);
    }

    // check_meter_validity with words not in dict
    SECTION("check_meter_validity error handling") {
        std::string text_with_bad_word = "topple Qwerdag ruin Jasdfz";
        std::string meter = "/x /x /x";
        Rhyme_and_Meter::Check_Validity_Result result = dict.check_meter_validity(text_with_bad_word, meter);
        REQUIRE(result.unrecognized_words[0] == "Qwerdag");
        REQUIRE(result.unrecognized_words[1] == "Jasdfz");
        REQUIRE(result.is_valid == false);
    }

    // syllable checker
    SECTION("check_syllable_validity") {
        std::string text = "fire crime";
        std::string bad_text = "Qwortextant metropolis";
        int syllable_count_good = 3;
        int syllable_count_good2 = 2;
        int syllable_count_wrong = 4;
        REQUIRE(dict.check_syllable_validity(text, syllable_count_good).is_valid == true);
        REQUIRE(dict.check_syllable_validity(text, syllable_count_good2).is_valid == true);
        REQUIRE(dict.check_syllable_validity(text, syllable_count_wrong).is_valid == false);
        REQUIRE(dict.check_syllable_validity(bad_text, syllable_count_good2).unrecognized_words[0] == "Qwortextant");
        REQUIRE(dict.check_syllable_validity(bad_text, syllable_count_good2).is_valid == false);
    }

    // Note: these exact values will change any time you change the weights of insertions/deletions, substituions, etc. SO it maybe makes more sense just to do comparisons. E.g. Distance between "kitten" and "sitting" and less than distance between "kitten" and "written".
    SECTION("levenshtein_distance") {
        // two consonant swaps
        std::string phones1 = "K IH1 T AH0 N";
        std::string phones2 = "S IH1 T IH0 NG";
        REQUIRE(levenshtein_distance(phones1, phones2) == 11);

        // two insertions
        phones1 = "B AA1 R K";
        phones2 = "B AA1 R K IH0 NG";
        REQUIRE(levenshtein_distance(phones1, phones2) == 20);

        // three insertions at beginning
        phones1 = "B AA1 R K";
        phones2 = "T R IY1 B AA1 R K";
        REQUIRE(levenshtein_distance(phones1, phones2) == 30);
    }

    SECTION("compare_end_line_rhyming_parts") {
        // P UH1 L IY0
        // B UH1 L IY0
        std::string line1 = "I pulled the pulley";
        std::string line2 = "which summoned by bully";
        auto rhyming_parts = dict.compare_end_line_rhyming_parts(line1, line2);
        REQUIRE(rhyming_parts.first[0]  == "UH1 L IY0");
        REQUIRE(rhyming_parts.second[0] == "UH1 L IY0" );

        line1 = "do you bleed";
        line2 = "Penelope";
        rhyming_parts = dict.compare_end_line_rhyming_parts(line1, line2);
        REQUIRE(rhyming_parts.first[0]  == "IY1 D");
        REQUIRE(rhyming_parts.second[0] == "IY0" );

        // USES  Y UW1 S AH0 Z
        // USES(1)  Y UW1 S IH0 Z
        // USES(2)  Y UW1 Z AH0 Z
        // USES(3)  Y UW1 Z IH0 Z
        // ABUSES  AH0 B Y UW1 S IH0 Z
        // ABUSES(1)  AH0 B Y UW1 Z IH0 Z
        line1 = "so many uses";
        line2 = "himself he abuses";
        rhyming_parts = dict.compare_end_line_rhyming_parts(line1, line2);
        REQUIRE(rhyming_parts.first.size() == 4);
        REQUIRE(rhyming_parts.second.size() == 2);
        REQUIRE(std::find(rhyming_parts.first.begin(), rhyming_parts.first.end(), "UW1 Z IH0 Z") != rhyming_parts.first.end());
        REQUIRE(std::find(rhyming_parts.first.begin(), rhyming_parts.first.end(), "UW1 S IH0 Z") != rhyming_parts.first.end());
        REQUIRE(std::find(rhyming_parts.second.begin(), rhyming_parts.second.end(), "UW1 S IH0 Z") != rhyming_parts.second.end());
        REQUIRE(std::find(rhyming_parts.second.begin(), rhyming_parts.second.end(), "UW1 Z IH0 Z") != rhyming_parts.second.end());
    }

    SECTION("minimum_end_rhyme_distance") {
        // perfect rhmye
        // UH1 L IY0
        std::string pulley = "I pulled the pulley";
        std::string bully = "which summoned by bully";
        int pulley_bully = dict.minimum_end_rhyme_distance(dict.compare_end_line_rhyming_parts(pulley, bully));
        REQUIRE(pulley_bully == 0);
        // Vowel Stress + Insertion
        // IY1 D
        // IY0
        std::string bleed = "do you bleed";
        std::string penelope = "Penelope";
        int bleed_penelope = dict.minimum_end_rhyme_distance(dict.compare_end_line_rhyming_parts(bleed, penelope));
        REQUIRE(bleed_penelope == GAP_PENALTY() + SUBSTITUTION_SCORE("IY1", "IY0"));
        // Vowel Distance + 1 Insertions
        // TODO check vowel stresses
        //   AO R    AH N JH
        // D AO R HH IH N JH
        // 
        std::string orange = "orange";
        std::string door_hinge = "door hinge";
        // TODO this fails because compare_end_line_rhyming_parts takes the shorter of the two options, i.e. one syllable hinge
        // preferred behavior is probably to take the length of the first term
        int orange_doorhinge = dict.minimum_end_rhyme_distance(dict.compare_end_line_rhyming_parts(orange, door_hinge));
        REQUIRE(orange_doorhinge == GAP_PENALTY() * 1 + SUBSTITUTION_SCORE("AH0", "IH1"));
        // Vowel Distance + 2 Insertions
        // UH1 L     IY0
        // AO1 L T R IY0
        pulley = "I pulled the pulley";
        std::string paltry = "that's so paltry";
        int pulley_paltry = dict.minimum_end_rhyme_distance(dict.compare_end_line_rhyming_parts(pulley, paltry));
        REQUIRE(pulley_paltry == GAP_PENALTY() * 2 + SUBSTITUTION_SCORE("UH1", "AO1"));
        // Perfect rhyme, but from among many pronunciations
        // USES  Y UW1 S AH0 Z
        // USES(1)  Y UW1 S IH0 Z
        // USES(2)  Y UW1 Z AH0 Z
        // USES(3)  Y UW1 Z IH0 Z
        // ABUSES  AH0 B Y UW1 S IH0 Z
        // ABUSES(1)  AH0 B Y UW1 Z IH0 Z
        std::string uses = "so many uses";
        std::string abuses = "himself he abuses";
        int uses_abuses = dict.minimum_end_rhyme_distance(dict.compare_end_line_rhyming_parts(uses, abuses));
        REQUIRE(uses_abuses == 0);
    }

    // this just strings the last two methods together
    SECTION("minimum_end_rhyme_distance") {
        std::string pulley = "I pulled the pulley";
        std::string bully = "which summoned by bully";
        int pulley_bully = dict.get_end_rhyme_distance(pulley, bully);
        REQUIRE(pulley_bully == 0);
    }
}

