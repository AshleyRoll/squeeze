#include <catch2/catch.hpp>
#include <squeeze/lib/priority_queue.h>
#include <functional>

using namespace squeeze;

SCENARIO("lib::priority_queue can add and remove items") {
    GIVEN("An empty min-priority queue") {
        lib::priority_queue<int, 5, std::greater<int>> q;

        REQUIRE(q.empty());
        REQUIRE(q.max_size() == 5);
        REQUIRE(q.size() == 0);

        WHEN("An item is added") {
            q.push(10);

            THEN("It should contain the right value") {
                REQUIRE_FALSE(q.empty());
                REQUIRE(q.size() == 1);
                REQUIRE(q.max_size() == 5);
                REQUIRE(q.top() == 10);
            }

            AND_WHEN("A second smaller value is added") {
                q.push(5);

                THEN("The top should be the new value") {
                    REQUIRE_FALSE(q.empty());
                    REQUIRE(q.size() == 2);
                    REQUIRE(q.max_size() == 5);
                    REQUIRE(q.top() == 5);
                }

                AND_WHEN("The smaller value is removed") {
                    q.pop();

                    THEN("The larger value should be at the top") {
                        REQUIRE_FALSE(q.empty());
                        REQUIRE(q.size() == 1);
                        REQUIRE(q.max_size() == 5);
                        REQUIRE(q.top() == 10);
                    }
                }
            }
        }
    }
}
