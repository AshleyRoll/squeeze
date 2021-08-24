#include <catch2/catch.hpp>
#include <string>

#include <squeeze/squeeze.h>

using namespace squeeze;
using Catch::Matchers::Equals;

static auto buildTableStrings = [] {
    return std::to_array<std::string_view> ({
        "First String",
        "Second String"
    });
};

SCENARIO("StringTable<NilEncoder> Can provide the correct strings", "[StringTable][NilEncoder]")
{
    GIVEN("A runtime initialised StringTable<NilEncoder>") {
        auto const table = StringTable<NilEncoder>(buildTableStrings);

        THEN("The number of strings should be correct") {
            REQUIRE(table.count() == 2);
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
