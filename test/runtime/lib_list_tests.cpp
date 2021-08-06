#include <catch2/catch.hpp>
#include <squeeze/lib/list.h>

using namespace squeeze;


SCENARIO("lib::list push_back / pop_back works") {
    GIVEN("An empty list") {
        lib::list<int> list;

        REQUIRE(list.empty());

        WHEN("An item is added") {
            list.push_back(1);

            THEN("It should contain the right value") {
                REQUIRE_FALSE(list.empty());
                REQUIRE(list.front() == 1);
                REQUIRE(list.back() == 1);
            }

            AND_WHEN("A second item is added") {
                list.push_back(2);

                THEN("It should contain the right values") {
                    REQUIRE_FALSE(list.empty());
                    REQUIRE(list.front() == 1);
                    REQUIRE(list.back() == 2);
                }

                AND_WHEN("The item is removed from the back") {
                    list.pop_back();
                    THEN("It should contain only 1 item"){
                        REQUIRE_FALSE(list.empty());
                        REQUIRE(list.front() == 1);
                        REQUIRE(list.back() == 1);
                    }
                }
            }
        }
    }
}

SCENARIO("lib::list push_front / pop_front works") {
    GIVEN("An empty list") {
        lib::list<int> list;

        REQUIRE(list.empty());

        WHEN("An item is added") {
            list.push_front(1);

            THEN("It should contain the right value") {
                REQUIRE_FALSE(list.empty());
                REQUIRE(list.front() == 1);
                REQUIRE(list.back() == 1);
            }

            AND_WHEN("A second item is added") {
                list.push_front(2);

                THEN("It should contain the right values") {
                    REQUIRE_FALSE(list.empty());
                    REQUIRE(list.front() == 2);
                    REQUIRE(list.back() == 1);
                }

                AND_WHEN("The item is removed from the front") {
                    list.pop_front();
                    THEN("It should contain only 1 item"){
                        REQUIRE_FALSE(list.empty());
                        REQUIRE(list.front() == 1);
                        REQUIRE(list.back() == 1);
                    }
                }
            }
        }
    }
}
