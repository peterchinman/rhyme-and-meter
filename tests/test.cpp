#include <catch2/catch_test_macros.hpp>
#include "convenience.h"
#include "CMU_Dict.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

struct DictFixture {
    mutable CMU_Dict dict;
    bool import_success{};

    DictFixture() {
        import_success = dict.import_dictionary();
    }
};


TEST_CASE_PERSISTENT_FIXTURE(DictFixture, "Dictionary tests") {
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
        std::string stresses = dict.phone_stresses("M AA1 D ER0 N AY2 Z D");
        REQUIRE(stresses == "102");
    }

    SECTION("word_to_stresses") {
        std::vector<std::string> stresses{dict.word_to_stresses("atoll")};
        REQUIRE(stresses[0] == "12");
        REQUIRE(stresses[2] == "01");
    }

    SECTION("phone_syllables") {
        REQUIRE(dict.phone_syllables("F AY1 ER0") == 2);
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
    }

    // this doesn't take into account optional meters...
    SECTION("meter_to_bool") {
        std::string meter{"x/x / x/x /x/"};
        std::vector<bool> binary = dict.meter_to_binary(meter);
        REQUIRE(binary.size() == 10);
        REQUIRE(binary[1] == 1);
        REQUIRE(binary[8] == 0);
    }

    SECTION("fuzzy_meter_to_set") {
        std::string meter{"x/x/x/x/(x/)"};
        std::set<std::vector<int>> meters_set = dict.fuzzy_meter_to_set(meter);
        REQUIRE(meters_set.size() == 2);
        for (auto & m : meters_set){
            std::cout << '{';
            for (auto & i : m) {
                std::cout << i << ", ";
            }
            std::cout << "}\n";
        }
        std::vector<int> check1 = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check1) != meters_set.end());
        std::vector<int> check2 = {0, 1, 0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check2) != meters_set.end());
    }

    SECTION("fuzzy_meter_to_set extra fuzzy edition") {
        std::string meter{"(x/)x/x/x/(x/)"};
        std::set<std::vector<int>> meters_set = dict.fuzzy_meter_to_set(meter);
        // 3 possibilities here because 2 of the possibilites are the same and set doesn't store duplicates
        REQUIRE(meters_set.size() == 3);
        std::vector<int> check1 = {0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check1) != meters_set.end());
        std::vector<int> check2 = {0, 1, 0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check2) != meters_set.end());
        std::vector<int> check3 = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
        REQUIRE(meters_set.find(check3) != meters_set.end());
    }



    // syllable checker
    // meter validity checker

    // SECTION("check_meter_validity single syllables") {
    //     std::string meter = "x/x/x/x/";
    //     std::string text = "I want to suck your blood right now."
    //     REQUIRE(dict.check_meter_validity(text, meter) == true);
    // }
}

