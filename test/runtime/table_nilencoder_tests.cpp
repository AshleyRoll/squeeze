#include <catch2/catch.hpp>
#include <string>

#include <squeeze/squeeze.h>

using namespace squeeze;
using Catch::Matchers::Equals;

auto buildTableStrings = [] {
    return std::to_array<std::string_view> ({
        "First String",
        "Second String"
    });
};

SCENARIO("StringTable<NilTableEncoder> Can provide the correct strings", "[StringTable][NilTableEncoder]")
{
    GIVEN("A runtime initialised StringTable<NilTableEncoder>") {
        auto const table = StringTable<NilTableEncoder>(buildTableStrings);

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
