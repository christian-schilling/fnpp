#include <cstdio>
#include <fn/synchronized.hpp>
#include <fn/optional.hpp>
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

        x >> [&](int& x) -> int& {
            REQUIRE(1 == lock_count);
            REQUIRE(0 == unlock_count);
            REQUIRE(5 == x);
            return x;
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

        x >> [&](int const& x) -> int const& {
            REQUIRE(1 == lock_count);
            REQUIRE(0 == unlock_count);
            REQUIRE(5 == x);
            return x;
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

        x >> [&](std::vector<int> const& x) -> std::vector<int> const& {
            REQUIRE(2 == lock_count);
            REQUIRE(1 == unlock_count);
            REQUIRE(0 == x.size());
            return x;
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

        int* addr1 = 0;
        int* addr2 = 0;

        {
            printf("0 #####\n");
            auto i = x >> [&](std::vector<int>& x) -> int&
            {
                printf("inside f\n");
                REQUIRE(2 == lock_count);
                REQUIRE(1 == unlock_count);
                REQUIRE(3 == x.size());
                addr1 = &x[0];
                return x[0];
            };
            printf("1 #####\n");

            REQUIRE(2 == lock_count);
            REQUIRE(1 == unlock_count);

            i >> [&](int& i) {
                addr2 = &i;
                REQUIRE(2 == lock_count);
                REQUIRE(1 == unlock_count);
                CHECK(1 == i);
                i = 9;
            };

            printf("2 #####\n");
        }

        CHECK(addr1 == addr2);
        CHECK(addr1 != 0);

        REQUIRE(2 == lock_count);
        REQUIRE(2 == unlock_count);

        CHECK(9 == (*x.guard())[0]);

        REQUIRE(3 == lock_count);
        REQUIRE(3 == unlock_count);

        REQUIRE(3 == y.size());

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
