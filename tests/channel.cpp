#include <cstdio>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"

#include <fn++.hpp>
#include <fn/channel.hpp>
using namespace fn;

TEST_CASE("Channel [int]")
{
    auto channel = Channel<int,3>();
    REQUIRE_FALSE(channel.rx.recv().valid());

    SECTION("send one message")
    {
        channel.tx.send(123);
        CHECK(123 == ~channel.rx.recv());
        REQUIRE_FALSE(channel.rx.recv().valid());
    }

    SECTION("send two messages")
    {
        CHECK(channel.tx.send(123));
        CHECK(channel.tx.send(124));
        CHECK(123 == ~channel.rx.recv());
        CHECK(124 == ~channel.rx.recv());
        CHECK_FALSE(channel.rx.recv().valid());
    }

    SECTION("send more messages than fit")
    {
        CHECK(channel.tx.send(123));
        CHECK(channel.tx.send(124));
        CHECK(channel.tx.send(125));
        CHECK_FALSE(channel.tx.send(126));
        CHECK(123 == ~channel.rx.recv());
        CHECK(124 == ~channel.rx.recv());
        CHECK(125 == ~channel.rx.recv());
        CHECK_FALSE(channel.rx.recv().valid());
    }

    SECTION("copy sender")
    {
        auto tx = channel.tx;

        CHECK(channel.tx.send(2));
        CHECK(tx.send(1));

        CHECK(2 == ~channel.rx.recv());
        CHECK(1 == ~channel.rx.recv());
        CHECK_FALSE(channel.rx.recv().valid());
    }

    SECTION("move receiver, then send")
    {
        auto rx = std::move(channel.rx);

        CHECK(channel.tx.send(1));

        CHECK_FALSE(channel.rx.recv().valid());

        CHECK(1 == ~rx.recv());
        CHECK_FALSE(rx.recv().valid());
    }

    SECTION("send, then move receiver")
    {
        CHECK(channel.tx.send(1));
        auto rx = std::move(channel.rx);

        CHECK_FALSE(channel.rx.recv().valid());

        CHECK(1 == ~rx.recv());
        CHECK_FALSE(rx.recv().valid());
    }

    SECTION("remove elements")
    {
        CHECK(channel.tx.send(1));
        CHECK(channel.tx.send(2));
        CHECK(channel.tx.send(3));

        channel.rx.remove_if([](int const& i) -> bool {
            return i % 2;
        });

        CHECK(2 == ~channel.rx.recv());
        CHECK_FALSE(channel.rx.recv().valid());
    }
}

TEST_CASE("Channel [unique_ptr]")
{
    auto channel = Channel<std::unique_ptr<int>,3>();
    REQUIRE_FALSE(channel.rx.recv().valid());

    SECTION("send one message")
    {
        auto up = std::unique_ptr<int>(new int);
        *up = 1234;

        channel.tx.send(std::move(up));
        CHECK(nullptr == up);

        auto r = channel.rx.recv();
        CHECK(r.valid());
        r >>[&](std::unique_ptr<int>& i){
            REQUIRE(i != nullptr);
            CHECK(1234 == *i);
        };

        REQUIRE_FALSE(channel.rx.recv().valid());
    }

    SECTION("move receiver, then send")
    {
        auto rx = std::move(channel.rx);

        CHECK(channel.tx.send(std::unique_ptr<int>(new int(1))));

        CHECK_FALSE(channel.rx.recv().valid());

        CHECK(rx.recv().valid());
        CHECK_FALSE(rx.recv().valid());
    }

    SECTION("send, then move receiver")
    {
        CHECK(channel.tx.send(std::unique_ptr<int>(new int(1))));
        auto rx = std::move(channel.rx);

        CHECK_FALSE(channel.rx.recv().valid());

        CHECK(rx.recv().valid());
        CHECK_FALSE(rx.recv().valid());
    }
}

TEST_CASE("Ring")
{
    fn_::Ring<int,3> queue;
    CHECK_FALSE(queue.pop());
    CHECK(queue.empty());

    SECTION("push once")
    {
        new (queue.push()) int(7);
        CHECK_FALSE(queue.empty());
        CHECK(7 == *queue.pop());
        CHECK_FALSE(queue.pop());
    }

    SECTION("push twice")
    {
        new (queue.push()) int(6);
        CHECK_FALSE(queue.empty());
        new (queue.push()) int(8);
        CHECK_FALSE(queue.empty());
        CHECK(6 == *queue.pop());
        CHECK_FALSE(queue.empty());
        CHECK(8 == *queue.pop());
        CHECK(queue.empty());
    }

    SECTION("push twice, twice")
    {
        new (queue.push()) int(6);
        new (queue.push()) int(8);
        CHECK(6 == *queue.pop());
        CHECK(8 == *queue.pop());

        new (queue.push()) int(2);
        new (queue.push()) int(3);
        CHECK(2 == *queue.pop());
        CHECK(3 == *queue.pop());
    }

    SECTION("overflow")
    {
        new (queue.push()) int(1);
        new (queue.push()) int(2);
        new (queue.push()) int(3);
        CHECK_FALSE(queue.push());
        CHECK_FALSE(queue.push());
        CHECK_FALSE(queue.push());
        CHECK(1 == *queue.pop());
        CHECK(2 == *queue.pop());
        CHECK(3 == *queue.pop());
        CHECK(queue.empty());
    }

};

TEST_CASE("Ring remove_if")
{
    fn_::Ring<int,5> queue;
    CHECK(queue.empty());

    new (queue.push()) int(1);
    new (queue.push()) int(2);
    new (queue.push()) int(3);
    new (queue.push()) int(4);

    SECTION("remove none")
    {
        queue.remove_if([](int const&) -> bool { return false; });

        CHECK(1 == *queue.pop());
        CHECK(2 == *queue.pop());
        CHECK(3 == *queue.pop());
        CHECK(4 == *queue.pop());
        CHECK(queue.empty());
    }

    SECTION("remove all")
    {
        queue.remove_if([](int const&) -> bool { return true; });

        CHECK(queue.empty());
    }

    SECTION("remove uneven")
    {
        queue.remove_if([](int const& x) -> bool { return x % 2; });

        CHECK(2 == *queue.pop());
        CHECK(4 == *queue.pop());
        CHECK(queue.empty());
    }
}
