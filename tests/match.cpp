#include <cstdio>
#include <fn/match.hpp>
#include <fn/express.hpp>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"
using namespace fn;

auto const smallest_perfect_number = when(_ == 6);

auto const judge = [](int x){ return match(x)(
    when(_ == 3)            >>= "half perfect",
    smallest_perfect_number >>= "perfect",
    default_to              >>= "not perfect"
);};


TEST_CASE("match")
{
    {
        auto const t = match(3)(
            when(_ == 3) >>= 5,
            when(6 == _) >>= 9,
            default_to >>= 0
        );
        REQUIRE(5 == t);
    }

    {
        auto const t = match(3)(
            when(_ == 1) >>= 8,
            when(_ == 3) >>= 5,
            when(_ == 3) >>= 6,
            when(6 == _) >>= 9,
            default_to >>= 0
        );
        REQUIRE(5 == t);
    }

    {
        REQUIRE(std::string("not perfect") == judge(1));
        REQUIRE(std::string("half perfect") == judge(3));
        REQUIRE(std::string("perfect") == judge(6));
        REQUIRE(std::string("not perfect") == judge(7));
    }

    {
        auto x = 8;
        auto const t = match(7)(
            when(_ == 7) >>[&](int i) { return x*i; },
            when(_ == 3) >>= 5,
            when(_ == 6) >>= 9,
            default_to >>= 0
        );
        REQUIRE(56 == t);
    }

    {
        auto const t = match(12)(
            when(_ == 3) >>= 5,
            when(_ == 6) >>= 9,
            default_to >>[](int i) { return i*3; }
        );
        REQUIRE(36 == t);
    }

    {
        auto const t = match(7)(
            when(_ == 3) >>= 5,
            when(_ == 6) >>= 9,
            default_to >>[](int i){
                printf("no match: %d\n",i);
                return 0;
            }
        );
        REQUIRE(0 == t);
    }

    {
        auto const f = [](int x){return match(x)(
            when(_ > 3 && _ < 7) >>= (_*2),
            default_to >>= 0
        );};
        REQUIRE(0  == f(0 ));
        REQUIRE(0  == f(1 ));
        REQUIRE(0  == f(2 ));
        REQUIRE(0  == f(3 ));
        REQUIRE(8  == f(4 ));
        REQUIRE(10 == f(5 ));
        REQUIRE(12 == f(6 ));
        REQUIRE(0  == f(7 ));
        REQUIRE(0  == f(8 ));
        REQUIRE(0  == f(9 ));
        REQUIRE(0  == f(10));
    }
}
