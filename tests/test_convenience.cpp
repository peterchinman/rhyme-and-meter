#include <catch2/catch_test_macros.hpp>
#include "convenience.hpp"
#include <vector>
#include <string>

TEST_CASE("convenience functions tests") {

    SECTION("phones_string_to_vector basic functionality") {
        std::string phones = "K EH2 R IY0 OW1 K IY0";
        auto result = phones_string_to_vector(phones);
        
        REQUIRE(result.size() == 7);
        REQUIRE(result[0] == "K");
        REQUIRE(result[1] == "EH2");
        REQUIRE(result[2] == "R");
        REQUIRE(result[3] == "IY0");
        REQUIRE(result[4] == "OW1");
        REQUIRE(result[5] == "K");
        REQUIRE(result[6] == "IY0");
    }

    SECTION("phones_string_to_vector single phoneme") {
        std::string phones = "K";
        auto result = phones_string_to_vector(phones);
        
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == "K");
    }

    SECTION("phones_string_to_vector empty string") {
        std::string phones = "";
        auto result = phones_string_to_vector(phones);
        
        REQUIRE(result.size() == 0);
    }

    SECTION("phones_string_to_vector multiple spaces") {
        std::string phones = "K    EH2   R   IY0";
        auto result = phones_string_to_vector(phones);
        
        REQUIRE(result.size() == 4);
        REQUIRE(result[0] == "K");
        REQUIRE(result[1] == "EH2");
        REQUIRE(result[2] == "R");
        REQUIRE(result[3] == "IY0");
    }

    SECTION("phones_string_to_vector leading and trailing spaces") {
        std::string phones = "  K EH2 R IY0  ";
        auto result = phones_string_to_vector(phones);
        
        REQUIRE(result.size() == 4);
        REQUIRE(result[0] == "K");
        REQUIRE(result[1] == "EH2");
        REQUIRE(result[2] == "R");
        REQUIRE(result[3] == "IY0");
    }

    SECTION("phones_vector_to_string basic functionality") {
        std::vector<std::string> phones = {"K", "EH2", "R", "IY0", "OW1", "K", "IY0"};
        auto result = phones_vector_to_string(phones);
        
        REQUIRE(result == "K EH2 R IY0 OW1 K IY0");
    }

    SECTION("phones_vector_to_string single phoneme") {
        std::vector<std::string> phones = {"K"};
        auto result = phones_vector_to_string(phones);
        
        REQUIRE(result == "K");
    }

    SECTION("phones_vector_to_string empty vector") {
        std::vector<std::string> phones = {};
        auto result = phones_vector_to_string(phones);
        
        REQUIRE(result == "");
    }

    SECTION("round trip conversion") {
        std::string original = "K EH2 R IY0 OW1 K IY0";
        auto vector_result = phones_string_to_vector(original);
        auto string_result = phones_vector_to_string(vector_result);
        
        REQUIRE(string_result == original);
    }

    SECTION("round trip conversion single phoneme") {
        std::string original = "K";
        auto vector_result = phones_string_to_vector(original);
        auto string_result = phones_vector_to_string(vector_result);
        
        REQUIRE(string_result == original);
    }
} 
