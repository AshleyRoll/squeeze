#include <catch2/catch.hpp>

#include <squeeze/squeeze.h>

using namespace squeeze;
using Catch::Matchers::Equals;

static auto buildTableStrings = [] {
    return std::to_array<std::string_view> ({
        "First String",
        "Second String"
    });
};

SCENARIO("StringTable<NilEncoder> can be compile-time initialised", "[StringTable][NilEncoder]") {
    GIVEN("A compile-time initialised StringTable<NilEncoder>"){
        static constinit auto table = StringTable<NilEncoder>(buildTableStrings);

        THEN("The number of strings should be correct"){
            STATIC_REQUIRE(table.count() == 2);
        }

        WHEN("The first string is retrieved") {
            // convert to strings so Catch can use them
            auto t = std::string{table[0]};

            THEN("The string should match the source data") {
                REQUIRE_THAT( t, Equals("First String") );
            }
        }

        WHEN("The second string is retrieved") {
            // convert to strings so Catch can use them
            auto t = std::string{table[1]};

            THEN("The string should match the source data") {
                REQUIRE_THAT( t, Equals("Second String") );
            }
        }

        WHEN("An invalid index is accessed") {
            THEN("An exception should be thrown") {
                REQUIRE_THROWS(table[3]);
            }
        }
    }
}
