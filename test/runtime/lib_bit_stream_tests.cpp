#include <catch2/catch.hpp>
#include <squeeze/lib/bit_stream.h>

using namespace squeeze;

SCENARIO("lib:bit_stream allocates expected storage") {
    GIVEN("a bit_stream of various sizes") {
        lib::bit_stream<1> bs1;
        lib::bit_stream<2> bs2;
        lib::bit_stream<3> bs3;
        lib::bit_stream<4> bs4;
        lib::bit_stream<5> bs5;
        lib::bit_stream<6> bs6;
        lib::bit_stream<7> bs7;
        lib::bit_stream<8> bs8;
        lib::bit_stream<9> bs9;

        THEN("the sizes should match to nearest byte") {
            REQUIRE(sizeof(bs1) == 1);
            REQUIRE(sizeof(bs2) == 1);
            REQUIRE(sizeof(bs3) == 1);
            REQUIRE(sizeof(bs4) == 1);
            REQUIRE(sizeof(bs5) == 1);
            REQUIRE(sizeof(bs6) == 1);
            REQUIRE(sizeof(bs7) == 1);
            REQUIRE(sizeof(bs8) == 1);
            REQUIRE(sizeof(bs9) == 2);
        }
    }
}

SCENARIO("lib:bit_stream can set and clear bits") {
    GIVEN("A bit_stream with 1 bit") {
        lib::bit_stream<1> bs1;

        WHEN("The first bit is set"){
            bs1.set(0);

            THEN("we should see it set") {
                REQUIRE(bs1.at(0) == true);
            }

            AND_WHEN("we clear the first bit") {
                bs1.clear(0);

                THEN(" we should see it clear") {
                    REQUIRE(bs1.at(0) == false);
                }
            }
        }
    }

    GIVEN("a bit_stream with 8 bits") {
        lib::bit_stream<8> bs8;

        WHEN("The first bit is set"){
            bs8.set(0);

            THEN("we should see it set, and no others set") {
                REQUIRE(bs8.at(0) == true);
                REQUIRE(bs8.at(1) == false);
                REQUIRE(bs8.at(2) == false);
                REQUIRE(bs8.at(3) == false);
                REQUIRE(bs8.at(4) == false);
                REQUIRE(bs8.at(5) == false);
                REQUIRE(bs8.at(6) == false);
                REQUIRE(bs8.at(7) == false);
            }

            AND_WHEN("we clear the first bit") {
                bs8.clear(0);

                THEN(" we should see it clear, and no others set") {
                    REQUIRE(bs8.at(0) == false);
                    REQUIRE(bs8.at(1) == false);
                    REQUIRE(bs8.at(2) == false);
                    REQUIRE(bs8.at(3) == false);
                    REQUIRE(bs8.at(4) == false);
                    REQUIRE(bs8.at(5) == false);
                    REQUIRE(bs8.at(6) == false);
                    REQUIRE(bs8.at(7) == false);
                }
            }
        }

        WHEN("The last bit is set"){
            bs8.set(7);

            THEN("we should see it set, and no others set") {
                REQUIRE(bs8.at(0) == false);
                REQUIRE(bs8.at(1) == false);
                REQUIRE(bs8.at(2) == false);
                REQUIRE(bs8.at(3) == false);
                REQUIRE(bs8.at(4) == false);
                REQUIRE(bs8.at(5) == false);
                REQUIRE(bs8.at(6) == false);
                REQUIRE(bs8.at(7) == true);
            }

            AND_WHEN("we clear the first bit") {
                bs8.clear(7);

                THEN(" we should see it clear, and no others set") {
                    REQUIRE(bs8.at(0) == false);
                    REQUIRE(bs8.at(1) == false);
                    REQUIRE(bs8.at(2) == false);
                    REQUIRE(bs8.at(3) == false);
                    REQUIRE(bs8.at(4) == false);
                    REQUIRE(bs8.at(5) == false);
                    REQUIRE(bs8.at(6) == false);
                    REQUIRE(bs8.at(7) == false);
                }
            }
        }

    }
}
