#include <cstdio>
#include <fn/shared.hpp>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"
using namespace fn;


TEST_CASE("An shared value")
{
    auto i = std::unique_ptr<shared<int>>(new shared<int>(5));
    CHECK(5 == *i);

    *i = 6;
    CHECK(6 == *i);

    (*i)++;
    CHECK(7 == *i);

    ref<int> r(*i);
    ref<int> r2(r);

    CHECK(7 == ~r);
    CHECK(7 == ~r2);

    (*i)++;

    CHECK(8 == ~r);
    CHECK(8 == ~r2);

    SECTION("delete"){
        i.reset(nullptr);
        CHECK_FALSE(r.valid());
        CHECK_FALSE(r2.valid());
    }

    SECTION("move"){
        auto j = std::move(*i);
        CHECK(r.valid());
        CHECK(r2.valid());

        (*i)++;
        CHECK(8 == ~r);
        CHECK(8 == ~r2);

        j++;
        CHECK(9 == ~r);
        CHECK(9 == ~r2);

        i.reset(nullptr);
        CHECK(9 == ~r);
        CHECK(9 == ~r2);
    }
}


TEST_CASE("chained")
{
    auto a = std::unique_ptr<fn_::chained>(new fn_::chained());

    CHECK(1 == a->count());

    auto b = *a;

    CHECK(2 == a->count());
    CHECK(2 == b.count());

    auto c = *a;

    CHECK(3 == a->count());
    CHECK(3 == b.count());
    CHECK(3 == c.count());

    SECTION("del")
    {
        auto d = b;

        CHECK(4 == a->count());
        CHECK(4 == b.count());
        CHECK(4 == c.count());
        CHECK(4 == d.count());
    }

    CHECK(3 == a->count());
    CHECK(3 == b.count());
    CHECK(3 == c.count());

    a.reset(nullptr);

    CHECK(2 == b.count());
    CHECK(2 == c.count());
}
