#include <catch2/catch_test_macros.hpp>
#include "rhyme_and_meter.hpp"
#include "Hirschberg.hpp"

struct Fixture {
    mutable Rhyme_and_Meter dict;
};

TEST_CASE_PERSISTENT_FIXTURE(Fixture, "Dictionary tests") {
   SECTION("hirschberg") {
      
   }
}
