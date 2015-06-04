#include <cstdio>
#include <fn++.hpp>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"
using namespace fn;



TEST_CASE("Express")
{
    SECTION("simple_expressions")
    {
        REQUIRE((_ == 4)(4));
        REQUIRE_FALSE((_ == 4)(5));
        REQUIRE((4 == _)(4));
        REQUIRE_FALSE((4 == _)(5));

        REQUIRE_FALSE((_ != 4)(4));
        REQUIRE((_ != 4)(5));
        REQUIRE_FALSE((4 != _)(4));
        REQUIRE((4 != _)(5));

        REQUIRE((4 < _)(8));
        REQUIRE_FALSE((4 < _)(4));
        REQUIRE_FALSE((4 < _)(1));
        REQUIRE((_ < 5)(1));
        REQUIRE_FALSE((_ < 5)(8));
        REQUIRE_FALSE((_ < 5)(5));

        REQUIRE_FALSE((4 > _)(8));
        REQUIRE_FALSE((4 > _)(4));
        REQUIRE((4 > _)(1));
        REQUIRE_FALSE((_ > 5)(1));
        REQUIRE((_ > 5)(8));
        REQUIRE_FALSE((_ > 5)(5));

        REQUIRE((4 <= _)(8));
        REQUIRE((4 <= _)(4));
        REQUIRE_FALSE((4 <= _)(1));
        REQUIRE((_ <= 5)(1));
        REQUIRE_FALSE((_ <= 5)(8));
        REQUIRE((_ <= 5)(5));

        REQUIRE_FALSE((4 >= _)(8));
        REQUIRE((4 >= _)(4));
        REQUIRE((4 >= _)(1));
        REQUIRE_FALSE((_ >= 5)(1));
        REQUIRE((_ >= 5)(8));
        REQUIRE((_ >= 5)(5));

        REQUIRE(10 == (3 + _)(7));
    }
    SECTION("unary_operators")
    {
        REQUIRE_FALSE((!_)(true));
        REQUIRE((!_)(false));
        REQUIRE(~0xf0 == (~_)(0xf0));
    }

    SECTION("complex_expressions")
    {
        REQUIRE(8 == (_)(8));

        REQUIRE(16 == (_ + _)(8));
        REQUIRE(12 == (_ + 4)(8));
        REQUIRE(13 == (5 + _)(8));
        REQUIRE(14 == ((5 + _)*_)(2));
        REQUIRE(4 == (_*_)(2));

        auto x = (10 == (3 + _));
        REQUIRE(true == x(7));
        REQUIRE(12 ==  (2 + (3 + _))(7));
        REQUIRE(false ==  (10 == (3 + _))(3));

        auto e =  4 & (1 << _);
        REQUIRE(e(2));
        REQUIRE_FALSE(e(3));

        REQUIRE(3 == (8 % _)(5));

        auto s = std::string("hello") + _;

        REQUIRE("hello world" == s(" world"));
    }

    SECTION("in")
    {
        REQUIRE_FALSE(_.in()(8));
        REQUIRE_FALSE(_.in()(9));

        REQUIRE(_.in(8)(8));
        REQUIRE_FALSE(_.in(8)(9));

        REQUIRE(_.in(8,7)(8));
        REQUIRE_FALSE(_.in(8,7)(9));
    }
}
