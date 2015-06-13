#include <cstdio>
#include <vector>
#include <map>
#include <utility>
#include <thread>
#include "catch.hpp"

#include <fn++.hpp>
#include <fn/channel.hpp>
using namespace fn;

TEST_CASE("Channel [int]")
{
    auto channel = Channel<int>(3);
    REQUIRE_FALSE(channel.receive(0).valid());

    SECTION("send one message")
    {
        channel.send(123);
        CHECK(123 == ~channel.receive(0));
        REQUIRE_FALSE(channel.receive(0).valid());
    }

    SECTION("send two messages")
    {
        CHECK(channel.send(123));
        CHECK(channel.send(124));
        CHECK(123 == ~channel.receive(0));
        CHECK(124 == ~channel.receive(0));
        CHECK_FALSE(channel.receive(0).valid());
    }

    SECTION("send more messages than fit")
    {
        CHECK(channel.send(123));
        CHECK(channel.send(124));
        CHECK(channel.send(125));
        CHECK_FALSE(channel.send(126));
        CHECK(123 == ~channel.receive(0));
        CHECK(124 == ~channel.receive(0));
        CHECK(125 == ~channel.receive(0));
        CHECK_FALSE(channel.receive(0).valid());
    }

    SECTION("copy sender")
    {
        auto send = channel.send;

        CHECK(channel.send(2));
        CHECK(send(1));

        CHECK(2 == ~channel.receive(0));
        CHECK(1 == ~channel.receive(0));
        CHECK_FALSE(channel.receive(0).valid());
    }

    SECTION("move receiver, then send")
    {
        auto receive = std::move(channel.receive);

        CHECK(channel.send(1));

        CHECK_FALSE(channel.receive(0).valid());

        CHECK(1 == ~receive(0));
        CHECK_FALSE(receive(0).valid());
    }

    SECTION("send, then move receiver")
    {
        CHECK(channel.send(1));
        auto receive = std::move(channel.receive);

        CHECK_FALSE(channel.receive(0).valid());

        CHECK(1 == ~receive(0));
        CHECK_FALSE(receive(0).valid());
    }

    SECTION("remove elements")
    {
        CHECK(channel.send(1));
        CHECK(channel.send(2));
        CHECK(channel.send(3));

        channel.receive.remove_if([](int const& i) -> bool {
            return i % 2;
        });

        CHECK(2 == ~channel.receive(0));
        CHECK_FALSE(channel.receive(0).valid());
    }

    SECTION("send one message to thread, receive gets called after send")
    {
        auto thread = std::thread([&]{
            CHECK(123 == ~channel.receive(10000));
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        channel.send(123);
        thread.join();
    }

    SECTION("send one message to thread, receive gets called before send")
    {
        auto thread = std::thread([&]{
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            CHECK(123 == ~channel.receive(10000));
        });
        channel.send(123);
        thread.join();
    }
}

TEST_CASE("Channel [unique_ptr]")
{
    auto channel = Channel<std::unique_ptr<int>>(3);
    REQUIRE_FALSE(channel.receive(0).valid());

    SECTION("send one message")
    {
        auto up = std::unique_ptr<int>(new int);
        *up = 1234;

        channel.send(std::move(up));
        CHECK(nullptr == up);

        auto r = channel.receive(0);
        CHECK(r.valid());
        r >>[&](std::unique_ptr<int>& i){
            REQUIRE(i != nullptr);
            CHECK(1234 == *i);
        };

        REQUIRE_FALSE(channel.receive(0).valid());
    }

    SECTION("move receiver, then send")
    {
        auto receive = std::move(channel.receive);

        CHECK(channel.send(std::unique_ptr<int>(new int(1))));

        CHECK_FALSE(channel.receive(0).valid());

        CHECK(receive(0).valid());
        CHECK_FALSE(receive(0).valid());
    }

    SECTION("send, then move receiver")
    {
        CHECK(channel.send(std::unique_ptr<int>(new int(1))));
        auto receive = std::move(channel.receive);

        CHECK_FALSE(channel.receive(0).valid());

        CHECK(receive(0).valid());
        CHECK_FALSE(receive(0).valid());
    }
}

TEST_CASE("Ring")
{
    auto queue = fn_::Ring<int>::create(3);

    CHECK_FALSE(queue->pop());
    CHECK(queue->empty());

    SECTION("push once")
    {
        new (queue->push()) int(7);
        CHECK_FALSE(queue->empty());
        CHECK(7 == *queue->pop());
        CHECK_FALSE(queue->pop());
    }

    SECTION("push twice")
    {
        new (queue->push()) int(6);
        CHECK_FALSE(queue->empty());
        new (queue->push()) int(8);
        CHECK_FALSE(queue->empty());
        CHECK(6 == *queue->pop());
        CHECK_FALSE(queue->empty());
        CHECK(8 == *queue->pop());
        CHECK(queue->empty());
    }

    SECTION("push twice, twice")
    {
        new (queue->push()) int(6);
        new (queue->push()) int(8);
        CHECK(6 == *queue->pop());
        CHECK(8 == *queue->pop());

        new (queue->push()) int(2);
        new (queue->push()) int(3);
        CHECK(2 == *queue->pop());
        CHECK(3 == *queue->pop());
    }

    SECTION("overflow")
    {
        new (queue->push()) int(1);
        new (queue->push()) int(2);
        new (queue->push()) int(3);
        CHECK_FALSE(queue->push());
        CHECK_FALSE(queue->push());
        CHECK_FALSE(queue->push());
        CHECK(1 == *queue->pop());
        CHECK(2 == *queue->pop());
        CHECK(3 == *queue->pop());
        CHECK(queue->empty());
    }

};

struct M {
    int value;
    static int counter;

    M(int value): value(value) { ++counter; }
    ~M() { --counter; }

    M(M const&) = delete;
    M(M&& m): value(m.value) { ++counter; }
};

int M::counter = 0;

TEST_CASE("Ring remove_if")
{
    M::counter = 0;

    auto queue = fn_::Ring<M>::create(5);

    CHECK(queue->empty());
    CHECK(0 == M::counter);

    new (queue->push()) M(1);
    new (queue->push()) M(2);
    new (queue->push()) M(3);
    new (queue->push()) M(4);

    CHECK(4 == M::counter);

    SECTION("remove none")
    {
        queue->remove_if([](M const&) -> bool { return false; });

        CHECK(1 == queue->pop()->value);
        CHECK(2 == queue->pop()->value);
        CHECK(3 == queue->pop()->value);
        CHECK(4 == queue->pop()->value);
        CHECK(queue->empty());
        CHECK(4 == M::counter);
    }

    SECTION("remove all")
    {
        queue->remove_if([](M const&) -> bool { return true; });

        CHECK(queue->empty());
        CHECK(0 == M::counter);
    }

    SECTION("remove uneven")
    {
        queue->remove_if([](M const& x) -> bool { return x.value % 2; });

        CHECK(2 == queue->pop()->value);
        CHECK(4 == queue->pop()->value);
        CHECK(queue->empty());
        CHECK(2 == M::counter);
    }
}
