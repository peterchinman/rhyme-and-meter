#include <catch2/catch_test_macros.hpp>
#include "consonant_distance.hpp"

#include <optional>

struct ConsonantFixture {
    ConsonantFixture() {
        ConsonantDistance::initialize();
    }
};

TEST_CASE_PERSISTENT_FIXTURE(ConsonantFixture, "consonant_distance tests") {

   SECTION("consonant_distance initialize get_consonant"){
      Consonant b{ConsonantDistance::get_consonant("B")};
      REQUIRE(b.manner == Manner::plosive);
      REQUIRE(b.sibilant == false);
      REQUIRE(b.voiced == true);
      REQUIRE(b.place == 1);
   }

   // get_consonant() throws an exception if it can't find the consonant
   SECTION("consonant_distance get_consonant exception"){
      REQUIRE_THROWS(ConsonantDistance::get_consonant("QS"));
   }

   SECTION("get_distance"){
      // BOTH PLOSIVE
      std::string phone1 = "B";
      std::string phone2 = "K";
      // delta_place of 6 + voiced_penalty
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == 6 + HARDCODED::VOICED_PENALTY);

      // BOTH FRICATIVE

      phone1 = "F";
      phone2 = "HH";
      // delta_place of 6
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == 6);

      // BOTH AFFRICATE
      phone1 = "CH";
      phone2 = "JH";
      // delta_place 0 + voiced_penalty
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == HARDCODED::VOICED_PENALTY);

      // BOTH NASAL

      phone1 = "M";
      phone2 = "NG";
      // delta_place 6
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == 6);

      // R & L 
      phone1 = "R";
      phone2 = "L";
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == HARDCODED::R_L_DISTANCE);

      // W & V
      phone1 = "W";
      phone2 = "V";
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == HARDCODED::W_V_DISTANCE);

      // APPROXIMANTS
      phone1 = "W";
      phone2 = "L";
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == 5);

      // ONE AFFRICATE ONE SIBILANT FRICATIVE
      phone1 = "CH";
      phone2 = "SH";
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == HARDCODED::AFFRICATE_SIBILANT_FRICATIVE_PENALTY);

      // ONE AFFRICATE ONE NON SIBILANT FRICATIVE
      phone1 = "CH";
      phone2 = "TH";
      // delta_place 2
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == HARDCODED::AFFRICATE_NON_SIBILANT_FRICATIVE_PENALTY + 2);

      // ONE AFFRICATE ONE PLOSIVE
      phone1 = "CH";
      phone2 = "T";
      // delta_place 1
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == HARDCODED::AFFRICATE_PLOSIVE_PENALTY + 1);

      // UNRELATED CONSONANTS
      phone1 = "M";
      phone2 = "F";
      REQUIRE(ConsonantDistance::get_distance(phone1, phone2) == HARDCODED::UNRELATED_CONSONANT_PENALTY);

   }

    
}
