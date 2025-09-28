#include <catch2/catch_test_macros.hpp>
#include "distance.hpp"
#include "levenshtein_distance.hpp"
#include "rhyme_and_meter.hpp"
#include "vowel_hex_graph.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

struct Fixture {
    mutable Rhyme_and_Meter dict;
    mutable VowelHexGraph vowel_hex_graph{};
    mutable ConsonantDistance Consonant_Distance{};
    // bool import_success{};

    // Fixture() {
        // bool import_success = dict.import_dictionary();
        // vowel_hex_graph.initialize();
    // }
};


TEST_CASE_PERSISTENT_FIXTURE(Fixture, "Dictionary tests") {

    SECTION("word_to_phones wrapper"){
        std::string word("karaoke");
        auto phones_vec = dict.word_to_phones(word);
        REQUIRE(phones_vec.has_value());
        REQUIRE(phones_vec.value()[0] == "K EH2 R IY0 OW1 K IY0");
    }

    SECTION("fuzzy_meter_to_binary_set") {
        std::string meter = "x/x";
        auto meters_set_result = dict.fuzzy_meter_to_binary_set(meter);
        REQUIRE(meters_set_result.has_value());
        const auto& meters_set = meters_set_result.value();
        REQUIRE(meters_set.size() == 1);
        REQUIRE(meters_set.find(std::vector<int>{0,1,0}) != meters_set.end());
    }

    SECTION("fuzzy_meter_to_binary_set extra fuzzy edition") {
        std::string meter{"(x/)x/(x/)"};
        auto meters_set_result = dict.fuzzy_meter_to_binary_set(meter);
        REQUIRE(meters_set_result.has_value());
        const auto& meters_set = meters_set_result.value();
        // 3 possibilities here because 2 of the possibilites are  the same and set doesn't store duplicates
        REQUIRE(meters_set.size() == 3);
        std::vector<int> check1 = {0, 1};
        REQUIRE(meters_set.find(check1) != meters_set.end());
        std::vector<int> check2 = {0, 1, 0, 1};
        REQUIRE(meters_set.find(check2) != meters_set.end());
        std::vector<int> check3 = {0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check3) != meters_set.end());

        // extra convoluted example
        std::string meter2 = "(/x)/x (/)x/(x)";
        auto meters_set_result2 = dict.fuzzy_meter_to_binary_set(meter2);
        REQUIRE(meters_set_result2.has_value());
        const auto& meters_set2 = meters_set_result2.value();
        std::vector<int> check4 = {1, 0, 1, 0, 1, 0, 1, 0};
        REQUIRE(meters_set2.size() == 8);
        REQUIRE(meters_set2.find(check4) != meters_set2.end());
    }

    SECTION("fuzzy_meter_to_binary_set error handling") {
        std::string meter = "x/x/(x/";
        auto meters_set_result = dict.fuzzy_meter_to_binary_set(meter);
        REQUIRE(!meters_set_result.has_value());
        REQUIRE(meters_set_result.error() == MeterError::UnclosedOptional);

        meter = "x/x/)x/";
        meters_set_result = dict.fuzzy_meter_to_binary_set(meter);
        REQUIRE(!meters_set_result.has_value());
        REQUIRE(meters_set_result.error() == MeterError::UnclosedOptional);

        meter = "x/x/(x/(x/))";
        meters_set_result = dict.fuzzy_meter_to_binary_set(meter);
        REQUIRE(!meters_set_result.has_value());
        REQUIRE(meters_set_result.error() == MeterError::NestedOptional);

        meter = "xjx";
        meters_set_result = dict.fuzzy_meter_to_binary_set(meter);
        REQUIRE(!meters_set_result.has_value());
        REQUIRE(meters_set_result.error() == MeterError::UnrecognizedCharacter);
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

    SECTION("levenshtein_distance") {
        // two consonant swaps
        std::string phones1 = "K IH1 T AH0 N";
        std::string phones2 = "S IH1 T IH0 NG";
        REQUIRE(levenshtein_distance(phones1, phones2) ==
                ConsonantDistance::get_distance("K", "S")
                + VowelHexGraph::get_distance("AH", "IH") * CONSTANTS::VOWEL::COEFFICIENT
                + ConsonantDistance::get_distance("N", "NG"));
    }

    SECTION("dynamic_gap_penalty") {
        // Test that gap penalty varies based on vowel vs consonant
        // Vowel gap penalty should be higher than consonant gap penalty
        
        // Test 1: Insertion/deletion of vowel vs consonant
        // "K IH1 T" vs "K IH1 T AH0" - inserting a vowel (AH0)
        std::string phones1 = "K IH1 T";
        std::string phones2 = "K IH1 T AH0";
        int vowel_insertion_distance = levenshtein_distance(phones1, phones2);
        
        // "K IH1 T" vs "K IH1 T N" - inserting a consonant (N)
        std::string phones3 = "K IH1 T N";
        int consonant_insertion_distance = levenshtein_distance(phones1, phones3);
        
        // Vowel insertion should have higher penalty than consonant insertion
        REQUIRE(vowel_insertion_distance > consonant_insertion_distance);
        
        // Test 2: Deletion of vowel vs consonant
        // "K IH1 T AH0" vs "K IH1 T" - deleting a vowel (AH0)
        int vowel_deletion_distance = levenshtein_distance(phones2, phones1);
        
        // "K IH1 T N" vs "K IH1 T" - deleting a consonant (N)
        int consonant_deletion_distance = levenshtein_distance(phones3, phones1);
        
        // Vowel deletion should have higher penalty than consonant deletion
        REQUIRE(vowel_deletion_distance > consonant_deletion_distance);
        
        // Test 3: Verify specific penalty values
        // The vowel insertion should cost CONSTANTS::VOWEL::INDEL_PENALTY (15)
        REQUIRE(vowel_insertion_distance == CONSTANTS::VOWEL::INDEL_PENALTY);
        
        // The consonant insertion should cost CONSONANT::INDEL_PENALTY (8)
        REQUIRE(consonant_insertion_distance == CONSTANTS::CONSONANT::INDEL_PENALTY);
        
        // Test 4: Edge case - empty string vs single phoneme
        std::string empty = "";
        std::string single_vowel = "IH1";
        std::string single_consonant = "K";
        
        int empty_to_vowel = levenshtein_distance(empty, single_vowel);
        int empty_to_consonant = levenshtein_distance(empty, single_consonant);
        
        // Empty to vowel should cost vowel penalty
        REQUIRE(empty_to_vowel == CONSTANTS::VOWEL::INDEL_PENALTY);
        // Empty to consonant should cost consonant penalty
        REQUIRE(empty_to_consonant == CONSTANTS::CONSONANT::INDEL_PENALTY);
        
        // Test 7: Verify that the default gap penalty (no phoneme provided) uses consonant penalty
        // This tests the backward compatibility
        REQUIRE(GAP_PENALTY() == CONSTANTS::CONSONANT::INDEL_PENALTY);
    }

    SECTION("repeated_consonant_penalty") {
        // Test 1: Basic repeated consonant detection
        // "P UH1 L IY0" vs "F UH1 L L IY0" - "lee" has repeated L
        std::string phones1 = "P UH1 L IY0";  // "pulley"
        std::string phones2 = "P UH1 L L IY0"; // "pull lee" with repeated L
        int distance_with_repetition = levenshtein_distance(phones1, phones2);
        
        std::string phones3 = "F UH1 L M IY0"; // "full mee" with different consonant
        int distance_without_repetition = levenshtein_distance(phones1, phones3);
        
        // Distance with repetition should be less than distance without repetition
        REQUIRE(distance_with_repetition < distance_without_repetition);
        
        REQUIRE(distance_with_repetition == CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY);
        
        
        // Test 3: Adjacent repetition only (not distant repetition)
        // "K IH1 T" vs "K IH1 T T" - adjacent T repetition
        std::string phones4 = "K IH1 T";
        std::string phones5 = "K IH1 T T";
        int adjacent_repetition_distance = levenshtein_distance(phones4, phones5);
        
        // "K IH1 T" vs "K IH1 T K T" - non-adjacent T repetition (K in between)
        std::string phones6 = "K IH1 T K T";
        int non_adjacent_repetition_distance = levenshtein_distance(phones4, phones6);
        
        // Adjacent repetition should get reduced penalty, non-adjacent should not
        REQUIRE(adjacent_repetition_distance == CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY);
        REQUIRE(non_adjacent_repetition_distance == CONSTANTS::CONSONANT::INDEL_PENALTY * 2);
        
        // Test 4: Vowels should not get repetition penalty reduction
        // "K IH1 T" vs "K IH1 IH1 T" - repeated vowel
        std::string phones7 = "K IH1 IH1 T";
        int vowel_repetition_distance = levenshtein_distance(phones4, phones7);
        
        // Vowel repetition should still cost full vowel penalty
        REQUIRE(vowel_repetition_distance == CONSTANTS::VOWEL::INDEL_PENALTY);
        
        // Test 5: Repetition at boundaries
        // "L IY0" vs "L L IY0" - repetition at start
        std::string phones8 = "L IY0";
        std::string phones9 = "L L IY0";
        int start_repetition_distance = levenshtein_distance(phones8, phones9);
        REQUIRE(start_repetition_distance == CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY);
        
        // Test 6: Multiple adjacent repetitions
        // "K IH1 T" vs "K K IH1 T T" - multiple adjacent repetitions
        std::string phones11 = "K K IH1 T T";
        int multiple_repetition_distance = levenshtein_distance(phones4, phones11);
        
        // Should get reduced penalty for both K and T repetitions
        REQUIRE(multiple_repetition_distance == CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY * 2);
    }

    SECTION("compare_end_line_rhyming_parts") {
        // P UH1 L IY0
        // B UH1 L IY0
        std::string line1 = "I pulled the pulley";
        std::string line2 = "which summoned by bully";
        auto rhyming_parts = dict.compare_end_line_rhyming_parts(line1, line2);
        REQUIRE(rhyming_parts.has_value());
        auto parts = rhyming_parts.value();
        REQUIRE(!parts.first.empty());
        REQUIRE(!parts.second.empty());
        REQUIRE(parts.first.at(0)  == "UH1 L IY0");
        REQUIRE(parts.second.at(0) == "UH1 L IY0" );

        line1 = "do you bleed";
        line2 = "Penelope";
        rhyming_parts = dict.compare_end_line_rhyming_parts(line1, line2);
        REQUIRE(rhyming_parts.has_value());
        parts = rhyming_parts.value();
        REQUIRE(!parts.first.empty());
        REQUIRE(!parts.second.empty());
        REQUIRE(parts.first.at(0)  == "IY1 D");
        REQUIRE(parts.second.at(0) == "IY0" );

        // USES  Y UW1 S AH0 Z
        // USES(1)  Y UW1 S IH0 Z
        // USES(2)  Y UW1 Z AH0 Z
        // USES(3)  Y UW1 Z IH0 Z
        // ABUSES  AH0 B Y UW1 S IH0 Z
        // ABUSES(1)  AH0 B Y UW1 Z IH0 Z
        line1 = "so many uses";
        line2 = "himself he abuses";
        rhyming_parts = dict.compare_end_line_rhyming_parts(line1, line2);
        REQUIRE(rhyming_parts.has_value());
        parts = rhyming_parts.value();
        REQUIRE(std::find(parts.first.begin(), parts.first.end(), "UW1 Z IH0 Z") != parts.first.end());
        REQUIRE(std::find(parts.first.begin(), parts.first.end(), "UW1 S IH0 Z") != parts.first.end());
        REQUIRE(std::find(parts.second.begin(), parts.second.end(), "UW1 S IH0 Z") != parts.second.end());
        REQUIRE(std::find(parts.second.begin(), parts.second.end(), "UW1 Z IH0 Z") != parts.second.end());
    }

    SECTION("compare_end_line_rhyming_parts error cases") {
        // Test with non-existent words
        std::string line1 = "I pulled the xyzzy";
        std::string line2 = "which summoned the rrrzzz";
        auto rhyming_parts = dict.compare_end_line_rhyming_parts(line1, line2);
        REQUIRE(!rhyming_parts.has_value());
        REQUIRE(rhyming_parts.error().words.at(0) == "XYZZY");
        REQUIRE(rhyming_parts.error().words.at(1) == "RRRZZZ");
    }

    SECTION("minimum_rhyme_distance") {
        // perfect rhmye
        // UH1 L IY0
        std::string pulley = "I pulled the pulley";
        std::string bully = "which summoned by bully";
        auto rhyming_parts = dict.compare_end_line_rhyming_parts(pulley, bully);
        REQUIRE(rhyming_parts.has_value());
        auto pulley_bully = dict.minimum_rhyme_distance(rhyming_parts.value());
        REQUIRE(pulley_bully == 0);

        // Vowel Stress + Insertion
        // IY1 D
        // IY0
        std::string bleed = "do you bleed";
        std::string penelope = "Penelope";
        rhyming_parts = dict.compare_end_line_rhyming_parts(bleed, penelope);
        REQUIRE(rhyming_parts.has_value());
        auto bleed_penelope = dict.minimum_rhyme_distance(rhyming_parts.value());
        REQUIRE(bleed_penelope == GAP_PENALTY() + SUBSTITUTION_SCORE("IY1", "IY0"));

        // Vowel Distance + 1 Insertions
        // TODO check vowel stresses
        //   AO1 R    IH0 N JH
        // D AO1 R HH IH1 N JH
        std::string orange = "orange";
        std::string door_hinge = "door hinge";
        // TODO this fails because compare_end_line_rhyming_parts takes the shorter of the two options, i.e. one syllable  hinge
        // preferred behavior is probably to take the  length of the first term
        rhyming_parts = dict.compare_end_line_rhyming_parts(orange, door_hinge);
        REQUIRE(rhyming_parts.has_value());
        auto orange_doorhinge = dict.minimum_rhyme_distance(rhyming_parts.value());
        // REQUIRE(orange_doorhinge == GAP_PENALTY() * 1 + SUBSTITUTION_SCORE("AH0", "IH1"));

        // Vowel Distance + 2 Insertions
        // UH1 L     IY0
        // AO1 L T R IY0
        pulley = "I pulled the pulley";
        std::string paltry = "that's so paltry";
        rhyming_parts = dict.compare_end_line_rhyming_parts(pulley, paltry);
        REQUIRE(rhyming_parts.has_value());
        auto pulley_paltry = dict.minimum_rhyme_distance(rhyming_parts.value());
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
    rhyming_parts = dict.compare_end_line_rhyming_parts(uses, abuses);
        REQUIRE(rhyming_parts.has_value());
        auto uses_abuses = dict.minimum_rhyme_distance(rhyming_parts.value());
        REQUIRE(uses_abuses == 0);
    }

    SECTION("hirschberg") {
        std::string word1 = "kitten";
        std::string word2 = "sitting";
        auto maybe_pronunciations1{dict.word_to_phones(word1)};
        REQUIRE(maybe_pronunciations1.has_value());
        auto pronunciations1 = maybe_pronunciations1.value();

        auto maybe_pronunciations2{dict.word_to_phones(word2)};
        REQUIRE(maybe_pronunciations2.has_value());
        auto pronunciations2 = maybe_pronunciations2.value();

        auto alignment_and_distance(dict.minimum_alignment(std::make_pair(pronunciations1, pronunciations2)));

        REQUIRE(alignment_and_distance.distance == 
            ConsonantDistance::get_distance("K", "S")
            + VowelHexGraph::get_distance("AH", "IH") * CONSTANTS::VOWEL::COEFFICIENT
            + ConsonantDistance::get_distance("N", "NG")
        );
        // REQUIRE(alignment_and_distance.ZWpair.first.);
    }
    SECTION("hirschberg_dynamic_gap_penalty") {
        std::string word1 = "eye";
        std::string word2 = "my";
        auto maybe_pronunciations1{dict.word_to_phones(word1)};
        REQUIRE(maybe_pronunciations1.has_value());
        auto pronunciations1 = maybe_pronunciations1.value();

        auto maybe_pronunciations2{dict.word_to_phones(word2)};
        REQUIRE(maybe_pronunciations2.has_value());
        auto pronunciations2 = maybe_pronunciations2.value();

        auto alignment_and_distance(dict.minimum_alignment(std::make_pair(pronunciations1, pronunciations2)));

        REQUIRE(alignment_and_distance.distance == 
           CONSTANTS::CONSONANT::INDEL_PENALTY
        );

        std::string word3 = "eye";
        std::string word4 = "io";
        auto maybe_pronunciations3{dict.word_to_phones(word3)};
        REQUIRE(maybe_pronunciations3.has_value());
        auto pronunciations3 = maybe_pronunciations3.value();

        auto maybe_pronunciations4{dict.word_to_phones(word4)};
        REQUIRE(maybe_pronunciations4.has_value());
        auto pronunciations4 = maybe_pronunciations4.value();

        auto alignment_and_distance2(dict.minimum_alignment(std::make_pair(pronunciations3, pronunciations4)));

        REQUIRE(alignment_and_distance2.distance == 
           CONSTANTS::VOWEL::INDEL_PENALTY
        );
    }

    SECTION("hirschberg_repeated_consonant_penalty") {
        // Test 1: Basic repeated consonant detection
        // "P UH1 L IY0" vs "F UH1 L L IY0" - "lee" has repeated L
        std::vector<std::string> phones1 = {"P", "UH1", "L", "IY0"};  // "pulley"
        std::vector<std::string> phones2 = {"P", "UH1", "L", "L", "IY0"}; // "pull lee" with repeated L
        int distance_with_repetition = hirschberg(phones1, phones2).distance;
        
        std::vector<std::string> phones3 = {"F", "UH1", "L", "M", "IY0"}; // "full mee" with different consonant
        int distance_without_repetition = hirschberg(phones1, phones3).distance;
        
        // Distance with repetition should be less than distance without repetition
        REQUIRE(distance_with_repetition < distance_without_repetition);
        
        REQUIRE(distance_with_repetition == CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY);
        
        
        // Test 3: Adjacent repetition only (not distant repetition)
        // "K IH1 T" vs "K IH1 T T" - adjacent T repetition
        std::vector<std::string> phones4 = {"K", "IH1", "T"};
        std::vector<std::string> phones5 = {"K", "IH1", "T", "T"};
        int adjacent_repetition_distance = hirschberg(phones4, phones5).distance;
        
        // "K IH1 T" vs "K IH1 T K T" - non-adjacent T repetition (K in between)
        std::vector<std::string> phones6 = {"K", "IH1", "T", "K", "T"};
        int non_adjacent_repetition_distance = hirschberg(phones4, phones6).distance;
        
        // Adjacent repetition should get reduced penalty, non-adjacent should not
        REQUIRE(adjacent_repetition_distance == CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY);
        REQUIRE(non_adjacent_repetition_distance == CONSTANTS::CONSONANT::INDEL_PENALTY * 2);
        
        // Test 4: Vowels should not get repetition penalty reduction
        // "K IH1 T" vs "K IH1 IH1 T" - repeated vowel
        std::vector<std::string> phones7 = {"K", "IH1", "IH1", "T"};
        int vowel_repetition_distance = hirschberg(phones4, phones7).distance;
        
        // Vowel repetition should still cost full vowel penalty
        REQUIRE(vowel_repetition_distance == CONSTANTS::VOWEL::INDEL_PENALTY);
        
        // Test 5: Repetition at boundaries
        // "L IY0" vs "L L IY0" - repetition at start
        std::vector<std::string> phones8 = {"L", "IY0"};
        std::vector<std::string> phones9 = {"L", "L", "IY0"};
        int start_repetition_distance = hirschberg(phones8, phones9).distance;
        REQUIRE(start_repetition_distance == CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY);
        
        // Test 6: Multiple adjacent repetitions
        // "K IH1 T" vs "K K IH1 T T" - multiple adjacent repetitions
        std::vector<std::string> phones11 = {"K", "K", "IH1", "T", "T"};
        int multiple_repetition_distance = hirschberg(phones4, phones11).distance;
        
        // Should get reduced penalty for both K and T repetitions
        REQUIRE(multiple_repetition_distance == CONSTANTS::CONSONANT::REPEATED_CONSONANT_PENALTY * 2);
    }

    SECTION("get_end_rhyme_distance") {
        std::string pulley = "I pulled the pulley";
        std::string bully = "which summoned by bully";
        auto pulley_bully = dict.get_end_rhyme_distance(pulley, bully);
        REQUIRE(pulley_bully.has_value());
        REQUIRE(pulley_bully.value() == 0);
    }

    SECTION("get_end_rhyme_distance error cases") {
        // Test with non-existent words
        std::string line1 = "I pulled the xyzzy";
        std::string line2 = "which summoned by bully";
        auto result = dict.get_end_rhyme_distance(line1, line2);
        REQUIRE(!result.has_value());
        REQUIRE(result.error().words.at(0) == "XYZZY");

        // Test with empty lines
        line1 = "";
        line2 = "bully";
        result = dict.get_end_rhyme_distance(line1, line2);
        REQUIRE(!result.has_value());
    }

    SECTION("minimum_text_alignment") {
        // Test with words that have multiple pronunciations
        std::string text1 = "read";
        std::string text2 = "read";
        
        auto alignment_result = dict.minimum_text_alignment(text1, text2);
        REQUIRE(alignment_result.has_value());
        
        auto alignment = alignment_result.value();
        // Should find perfect alignment with distance 0
        REQUIRE(alignment.distance == 0);
        
        // Test with different words that should have some distance
        std::string text3 = "read";
        std::string text4 = "book";
        alignment_result = dict.minimum_text_alignment(text3, text4);
        REQUIRE(alignment_result.has_value());
        
        alignment = alignment_result.value();
        // Should have some distance > 0
        REQUIRE(alignment.distance > 0);
    }

    SECTION("minimum_text_alignment with multiple words") {
        // Test with multiple words
        std::string text1 = "read book";
        std::string text2 = "read book";
        
        auto alignment_result = dict.minimum_text_alignment(text1, text2);
        REQUIRE(alignment_result.has_value());
        
        auto alignment = alignment_result.value();
        // Should find perfect alignment with distance 0
        REQUIRE(alignment.distance == 0);
        
        // Test with different multi-word texts
        std::string text3 = "read book";
        std::string text4 = "write story";
        alignment_result = dict.minimum_text_alignment(text3, text4);
        REQUIRE(alignment_result.has_value());
        
        alignment = alignment_result.value();
        // Should have some distance > 0
        REQUIRE(alignment.distance > 0);
    }

    SECTION("minimum_text_alignment error handling") {
        // Test with text containing unrecognized words
        std::string text1 = "read xyzzy";
        std::string text2 = "book";
        
        auto alignment_result = dict.minimum_text_alignment(text1, text2);
        REQUIRE(!alignment_result.has_value());
        REQUIRE(alignment_result.error().words.size() == 1);
        REQUIRE(alignment_result.error().words[0] == "XYZZY");
        
        // Test with both texts having unrecognized words
        std::string text3 = "read xyzzy";
        std::string text4 = "book rrrzzz";
        alignment_result = dict.minimum_text_alignment(text3, text4);
        REQUIRE(!alignment_result.has_value());
        REQUIRE(alignment_result.error().words.size() == 2);
        REQUIRE(std::find(alignment_result.error().words.begin(), alignment_result.error().words.end(), "XYZZY") != alignment_result.error().words.end());
        REQUIRE(std::find(alignment_result.error().words.begin(), alignment_result.error().words.end(), "RRRZZZ") != alignment_result.error().words.end());
    }

    SECTION("compare_text_pronunciations template function") {
        // Test with minimum distance function
        std::string text1 = "read";
        std::string text2 = "read";
        
        auto distance_result = dict.minimum_text_distance(text1, text2);
        REQUIRE(distance_result.has_value());
        REQUIRE(distance_result.value() == 0);  // Same words should have distance 0
        
        // Test with different words
        std::string text3 = "read";
        std::string text4 = "book";
        distance_result = dict.minimum_text_distance(text3, text4);
        REQUIRE(distance_result.has_value());
        REQUIRE(distance_result.value() > 0);  // Different words should have distance > 0
    }

    // Commenting this out because I think this is not the best place to handle calibration
    // SECTION("phonetic_distance_calibration") {
    //     /**
    //      * We want to compare two pairs of word, where one word remains the same in each, e.g. A & B vs A & C.
    //      */

    //     // VOWEL COEFFICIENT TESTS (vowel change vs consonant change)
    //     // ball & bull vs ball & bog
    //     auto ball_bog = dict.minimum_text_distance("ball", "bog");
    //     auto ball_bull = dict.minimum_text_distance("ball", "bull");
    //     REQUIRE(ball_bog.has_value());
    //     REQUIRE(ball_bull.value() < ball_bog.value());
        
        
    //     // buck & book vs buck & bug
    //     auto buck_bug = dict.minimum_text_distance("buck", "bug");
    //     auto buck_book = dict.minimum_text_distance("buck", "book");
    //     REQUIRE(buck_bug.has_value());
    //     // Intuition: book should be closer to buck than bug (vowel change vs consonant change)
    //     REQUIRE(buck_book.value() > buck_bug.value());
        
    //     // tree & tray vs tree & treat
    //     auto tree_tray = dict.minimum_text_distance("tree", "tray");
    //     auto tree_treat = dict.minimum_text_distance("tree", "treat");
    //     REQUIRE(tree_tray.has_value());
    //     REQUIRE(tree_treat.has_value());
    //     // Intuition: tray should be closer to tree than treat (vowel change vs consonant change)
    //     REQUIRE(tree_tray.value() > tree_treat.value());
    // }
}
