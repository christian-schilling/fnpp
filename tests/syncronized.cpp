#include <cstdio>
#include <fn++.hpp>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"
using namespace fn;

static int lock_count = 0;
static int unlock_count = 0;

struct DummyMutex
{
    void lock(){ lock_count++; }
    void unlock(){ unlock_count++; }
};

TEST_CASE("synchronized")
{
    SECTION("calls_lock_and_unlock")
    {

        lock_count = 0;
        unlock_count = 0;
        auto x = synchronized<int,DummyMutex>(5);
        REQUIRE(0 == lock_count);
        REQUIRE(0 == unlock_count);

        with_(x){
            REQUIRE(1 == lock_count);
            REQUIRE(0 == unlock_count);
            REQUIRE(5 == x);
        };

        REQUIRE(1 == lock_count);
        REQUIRE(1 == unlock_count);
    }

    SECTION("works_with_const")
    {
        lock_count = 0;
        unlock_count = 0;
        auto const x = synchronized<int,DummyMutex>(5);
        REQUIRE(0 == lock_count);
        REQUIRE(0 == unlock_count);

        x >> [&](int const x){
            REQUIRE(1 == lock_count);
            REQUIRE(0 == unlock_count);
            REQUIRE(5 == x);
        };

        REQUIRE(1 == lock_count);
        REQUIRE(1 == unlock_count);
    }

    SECTION("take")
    {
        lock_count = 0;
        unlock_count = 0;
        auto x = synchronized<std::vector<int>,DummyMutex>(std::vector<int>{1,2,34});

        REQUIRE(0 == lock_count);
        REQUIRE(0 == unlock_count);

        auto y = x.take();

        REQUIRE(1 == lock_count);
        REQUIRE(1 == unlock_count);

        with_(x){
            REQUIRE(2 == lock_count);
            REQUIRE(1 == unlock_count);
            REQUIRE(0 == x.size());
        };

        REQUIRE(3 == y.size());

        REQUIRE(2 == lock_count);
        REQUIRE(2 == unlock_count);

    }

    SECTION("clone")
    {
        lock_count = 0;
        unlock_count = 0;
        auto x = synchronized<std::vector<int>,DummyMutex>(std::vector<int>{1,2,34});

        REQUIRE(0 == lock_count);
        REQUIRE(0 == unlock_count);

        auto y = x.clone();

        REQUIRE(1 == lock_count);
        REQUIRE(1 == unlock_count);

        with_(x){
            REQUIRE(2 == lock_count);
            REQUIRE(1 == unlock_count);
            REQUIRE(3 == x.size());
        };

        REQUIRE(3 == y.size());

        REQUIRE(2 == lock_count);
        REQUIRE(2 == unlock_count);
    }

    SECTION("guard")
    {
        lock_count = 0;
        unlock_count = 0;
        auto x = synchronized<int,DummyMutex>(5);
        REQUIRE(0 == lock_count);
        REQUIRE(0 == unlock_count);

        {
            auto xg = x.guard();
            REQUIRE(1 == lock_count);
            REQUIRE(0 == unlock_count);
            REQUIRE(5 == *xg);
        };
        REQUIRE(1 == lock_count);
        REQUIRE(1 == unlock_count);

        REQUIRE(5 == *x.guard());
        REQUIRE(2 == lock_count);
        REQUIRE(2 == unlock_count);

        REQUIRE(5 == *x.guard());
        REQUIRE(3 == lock_count);
        REQUIRE(3 == unlock_count);
    }

    struct GuardedObject
    {
        int x;
        GuardedObject(int x): x(x) {}
    };

    SECTION("guard_object")
    {
        lock_count = 0;
        unlock_count = 0;
        auto x = synchronized<GuardedObject,DummyMutex>(8);
        REQUIRE(0 == lock_count);
        REQUIRE(0 == unlock_count);

        REQUIRE(8 == x.guard()->x);
        REQUIRE(1 == lock_count);
        REQUIRE(1 == unlock_count);

        x.guard()->x = 10;

        REQUIRE(10 == x.guard()->x);
        REQUIRE(3 == lock_count);
        REQUIRE(3 == unlock_count);
    }

    SECTION("guard_object_const")
    {
        lock_count = 0;
        unlock_count = 0;
        auto const x = synchronized<GuardedObject,DummyMutex>(8);
        REQUIRE(0 == lock_count);
        REQUIRE(0 == unlock_count);

        REQUIRE(8 == x.guard()->x);
        REQUIRE(1 == lock_count);
        REQUIRE(1 == unlock_count);

        REQUIRE(8 == (*x.guard()).x);
        REQUIRE(2 == lock_count);
        REQUIRE(2 == unlock_count);
    }
}
