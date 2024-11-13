#include <catch2/catch_test_macros.hpp>
#include "convenience.h"
#include "CMU_Dict.h"
#include "VowelHexGraph.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

struct Fixture {
    mutable CMU_Dict dict;
    mutable VowelHexGraph vowel_hex_graph{};
    bool import_success{};

    Fixture() {
        import_success = dict.import_dictionary();
        vowel_hex_graph.initialize();
    }
};


TEST_CASE_PERSISTENT_FIXTURE(Fixture, "Dictionary tests") {
    SECTION ("dictionary import success") {
        REQUIRE(import_success == true);
    }
    SECTION("find_phones") {
        std::vector<std::string> word_lower = dict.find_phones("associate");
        REQUIRE(word_lower.size() == 4);
        REQUIRE(word_lower[0] == "AH0 S OW1 S IY0 AH0 T");
        // upper case
        std::vector<std::string> word_upper = dict.find_phones("ASSOCIATE");
        REQUIRE(word_upper.size() == 4);
        REQUIRE(word_upper[0] == "AH0 S OW1 S IY0 AH0 T");
    }

    SECTION("find_phones exception") {
        std::vector<std::string> bad_word{};
        REQUIRE_THROWS(bad_word = dict.find_phones("sdfasdg"));
    } 

    SECTION("phone_stresses") {
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

    // this doesn't take into account optional meters...
    SECTION("meter_to_bool") {
        std::string meter{"x/x / x/x /x/"};
        std::vector<bool> binary = dict.meter_to_binary(meter);
        REQUIRE(binary.size() == 10);
        REQUIRE(binary[1] == 1);
        REQUIRE(binary[8] == 0);
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

    // TODO handle failures for example "x/ ) x/ ()"

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


    SECTION("rhyme test") {
        // PATCH  P AE1 CH
        std::string line1 {"his dog did run into a patch"};
        // CATCH  K AE1 CH
        std::string line2 {"to fetch the ball the boy failed catch"};

        REQUIRE(dict.check_end_rhyme_validity(line1, line2, 0, 0).is_valid == true);
    }

    // single-rhyme
    // rhyme, sublime
    // "feminine" rhyme
    // picky, tricky
    // amorous, glamorous / practical, tactical

    // Feminine and dactylic rhymes may also be realized as compound (or mosaic) rhymes (poet, know it).

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

