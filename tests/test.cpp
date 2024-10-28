#include <catch2/catch_test_macros.hpp>
#include "CMU_Dict.h"

TEST_CASE("import dictionary") {
    CMU_Dict dict;
    REQUIRE(dict.import_dictionary() == true);
}
