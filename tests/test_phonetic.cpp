#include <catch2/catch_test_macros.hpp>
#include "phonetic.hpp"
#include <string>
#include <vector>

struct Fixture {
    mutable Phonetic dict;
};

TEST_CASE_PERSISTENT_FIXTURE(Fixture, "Dictionary tests") {
    SECTION ("dictionary import success") {
    }
    SECTION("word_to_phones") {
        auto word_lower = dict.word_to_phones("associate");
        REQUIRE(word_lower.has_value());
        REQUIRE(word_lower.value().size() == 4);
        REQUIRE(word_lower.value()[0] == "AH0 S OW1 S IY0 AH0 T");
        // upper case
        auto word_upper = dict.word_to_phones("ASSOCIATE");
        REQUIRE(word_upper.has_value());
        REQUIRE(word_upper.value().size() == 4);
        REQUIRE(word_upper.value()[0] == "AH0 S OW1 S IY0 AH0 T");
    }

    SECTION("word_to_phones error") {
        auto bad_word = dict.word_to_phones("sdfasdg");
        REQUIRE(!bad_word.has_value());
        REQUIRE(bad_word.error().unidentified_word == "SDFASDG");
    } 

    SECTION("phones_to_stresses") {
        std::string stresses = dict.phones_to_stresses("M AA1 D ER0 N AY2 Z D");
        REQUIRE(stresses == "102");
    }

    SECTION("word_to_stresses") {
        auto stresses = dict.word_to_stresses("atoll");
        REQUIRE(stresses.has_value());
        REQUIRE(stresses.value()[0] == "12");
        REQUIRE(stresses.value()[2] == "01");
    }

    SECTION("phone_to_syllable_count") {
        REQUIRE(dict.phone_to_syllable_count("F AY1 ER0") == 2);
    }

    SECTION("word_to_syllable_counts") {
        auto fire = dict.word_to_syllable_counts("fire");
        REQUIRE(fire.has_value());
        REQUIRE(fire.value().size() == 2);
        REQUIRE(fire.value()[0] == 2);
        REQUIRE(fire.value()[1] == 1);
    }

     SECTION("text_to_phones simple") {
        std::string text{"smelly dog"};
        auto results = dict.text_to_phones(text);
        REQUIRE(results.words_with_pronunciations.size() == 2);
        REQUIRE(results.failed_words.empty());
        REQUIRE(!results.has_failures());
        
        REQUIRE(results.words_with_pronunciations[0].first == "smelly");
        REQUIRE(results.words_with_pronunciations[0].second[0] == "S M EH1 L IY0");
        REQUIRE(results.words_with_pronunciations[1].first == "dog");
        REQUIRE(results.words_with_pronunciations[1].second[0] == "D AO1 G");
    }
  
    SECTION("text_to_phones complex punct") {
        std::string text{"Smelly dog? Drip-dry--good dog."};
        auto results = dict.text_to_phones(text);
        REQUIRE(results.words_with_pronunciations.size() == 5);
        REQUIRE(results.failed_words.empty());
        REQUIRE(!results.has_failures());
        
        REQUIRE(results.words_with_pronunciations[0].first == "Smelly");
        REQUIRE(results.words_with_pronunciations[0].second[0] == "S M EH1 L IY0");
        REQUIRE(results.words_with_pronunciations[2].first == "Drip-dry");
        REQUIRE(results.words_with_pronunciations[2].second[0] == "D R IH1 P D R AY1");
    }

    SECTION("text_to_phones errors") {
        std::string text{"Smelly dog? asdfaga"};
        auto results = dict.text_to_phones(text);
        REQUIRE(results.words_with_pronunciations.size() == 3);
        REQUIRE(results.failed_words.size() == 1);
        REQUIRE(results.has_failures());
        
        REQUIRE(results.words_with_pronunciations[0].first == "Smelly");
        REQUIRE(results.words_with_pronunciations[1].first == "dog");
        REQUIRE(results.words_with_pronunciations[2].first == "asdfaga");
        REQUIRE(results.words_with_pronunciations[2].second.empty()); // Failed word has empty pronunciations
        
        REQUIRE(results.failed_words[0] == "ASDFAGA");
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
}

