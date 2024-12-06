#include <catch2/catch_test_macros.hpp>
#include "convenience.h"
#include "Distance.h"
#include "CMU_Dict.h"
#include "VowelHexGraph.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

struct Fixture {
    mutable CMU_Dict dict;
    mutable VowelHexGraph vowel_hex_graph{};
    // bool import_success{};

    // Fixture() {
        // bool import_success = dict.import_dictionary();
        // vowel_hex_graph.initialize();
    // }
};


TEST_CASE_PERSISTENT_FIXTURE(Fixture, "Dictionary tests") {
    // SECTION ("dictionary import success") {
    //     REQUIRE(import_success == true);
    // }
    SECTION("word_to_phones") {
        std::vector<std::string> word_lower = dict.word_to_phones("associate");
        REQUIRE(word_lower.size() == 4);
        REQUIRE(word_lower[0] == "AH0 S OW1 S IY0 AH0 T");
        // upper case
        std::vector<std::string> word_upper = dict.word_to_phones("ASSOCIATE");
        REQUIRE(word_upper.size() == 4);
        REQUIRE(word_upper[0] == "AH0 S OW1 S IY0 AH0 T");
    }

    SECTION("word_to_phones exception") {
        std::vector<std::string> bad_word{};
        REQUIRE_THROWS(bad_word = dict.word_to_phones("sdfasdg"));
    } 

    SECTION("phone_to_stress") {
        std::string stresses = dict.phone_to_stress("M AA1 D ER0 N AY2 Z D");
        REQUIRE(stresses == "102");
    }

    SECTION("word_to_stresses") {
        std::vector<std::string> stresses{dict.word_to_stresses("atoll")};
        REQUIRE(stresses[0] == "12");
        REQUIRE(stresses[2] == "01");
    }

    SECTION("phone_syllables") {
        REQUIRE(dict.phone_to_syllable_count("F AY1 ER0") == 2);
    }

    SECTION("word_to_syllables") {
        std::vector<int> fire = dict.word_to_syllables("fire");
        REQUIRE(fire.size() == 2);
        REQUIRE(fire[0] == 2);
        REQUIRE(fire[1] == 1);
    }

    SECTION("text_to_phones simple") {
        std::string text{"smelly dog"};
        std::vector<std::pair<std::vector<std::string>, bool>> phones{dict.text_to_phones(text)};
        REQUIRE(phones.size() == 2);
        REQUIRE(phones[0].first[0] == "S M EH1 L IY0");
        REQUIRE(phones[1].first[0] == "D AO1 G");
    }

    SECTION("strip_punctuation") {
        std::string text{"Smelly dog? Drip-dry—-good dog."};
        std::vector<std::string> words{strip_punctuation(text)};
        REQUIRE(words.size() == 5);
        REQUIRE(words[0] == "Smelly");
        REQUIRE(words[2] == "Drip-dry");

        // apostrophes
        std::string text2{"Can't--couldn't"};
        std::vector<std::string> words2{strip_punctuation(text2)};
        // REQUIRE(words2.size() == 5);
        REQUIRE(words2[0] == "Can't");
        REQUIRE(words2[1] == "couldn't");
    }
  

    SECTION("text_to_phones complex punct") {
        std::string text{"Smelly dog? Drip-dry--good dog."};
        std::vector<std::pair<std::vector<std::string>, bool>> phones{dict.text_to_phones(text)};
        REQUIRE(phones.size() == 5);
        REQUIRE(phones[0].first[0] == "S M EH1 L IY0");
        REQUIRE(phones[2].first[0] == "D R IH1 P D R AY1");
    }

    SECTION("text_to_phones exceptions") {
        std::string text{"Smelly dog? asdfaga"};
        std::vector<std::pair<std::vector<std::string>, bool>> phones{dict.text_to_phones(text)};
        REQUIRE(phones.size() == 3);
        REQUIRE(phones[0].first[0] == "S M EH1 L IY0");
        REQUIRE(phones[2].second == false);
        REQUIRE(phones[2].first[0] == "asdfaga");
    }

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
        CMU_Dict::Check_Validity_Result result = dict.check_meter_validity(text_with_bad_word, meter);
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

    SECTION("get_rhyming_part") {
        std::string phones = "M AO1 R F";
        REQUIRE(dict.get_rhyming_part(phones) == "AO1 R F");
        phones = "P UH1 L IY0";
        REQUIRE(dict.get_rhyming_part(phones) == "UH1 L IY0");
        phones = "P EH1 R AH0 L AH0 S";
        REQUIRE(dict.get_rhyming_part(phones) == "EH1 R AH0 L AH0 S");
        phones = "P EH1 R AH0 S K OW2 P";
        REQUIRE(dict.get_rhyming_part(phones) == "OW2 P");
        phones = "DH AH0";
        REQUIRE(dict.get_rhyming_part(phones) == "AH0");
        phones = "DH S K";
        REQUIRE(dict.get_rhyming_part(phones) == "");
    }

    SECTION("phones_string_to_vector") {
        std::string phones1 = "P UH1 L IY0";
        std::vector<std::string> symbol_vec {phones_string_to_vector(phones1)};
        REQUIRE(symbol_vec.size() == 4); 
        REQUIRE(symbol_vec[0] == "P");
        REQUIRE(symbol_vec[1] == "UH1");
        REQUIRE(symbol_vec[2] == "L");
        REQUIRE(symbol_vec[3] == "IY0");
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

    

    // TODO Search!
    // Search maybe belongs to the Random Word Generator??
    // Search for pronunciation
    // Search for stresses
    // Search for rhmye


    /**
    * syllabic: a rhyme in which the last syllable of each word sounds the same but does not       necessarily contain stressed vowels. (cleaver, silver, or pitter, patter; the final syllable of the words bottle and fiddle is /l/, a liquid consonant.)
    * imperfect (or near): a rhyme between a stressed and an unstressed syllable. (wing, caring)
    weak (or unaccented): a rhyme between two sets of one or more unstressed syllables. (hammer, carpenter)
    * semirhyme: a rhyme with an extra syllable on one word. (bend, ending)
    * forced (or oblique): a rhyme with an imperfect match in sound. (green, fiend; one, thumb)
    * assonance: matching vowels. (shake, hate) Assonance is sometimes referred to as slant rhymes, along with consonance.
    * consonance: matching consonants. (rabies, robbers)
    * half rhyme (or slant rhyme): matching final consonants. (hand , lend)
    * pararhyme: all consonants match. (tick, tock)
    * alliteration (or head rhyme): matching initial consonants. (ship, short)
    */

    // TODO other types Optionality,
        //e.g. "/x /x [xx, //]" would mean EITHER "/x /x xx" or "/x /x //"
        

    
}

