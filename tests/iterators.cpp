#include <cstdio>
#include <fn++.hpp>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"
using namespace fn;

TEST_CASE("A range")
{
    SECTION("goes from zero to the upper limit not including the limit")
    {
        std::vector<int> expected{0,1,2,3,4,5,6,7,8,9};
        auto it = range(10);
        auto count = 0;
        for(auto i: it){
            CHECK(expected[count] == i);
            count++;
        }
        CHECK(10 == count);
    }

    SECTION("goes from start to the upper limit not including the limit")
    {
        std::vector<int> expected{5,6,7,8,9};
        auto it = range(5,10);
        auto count = 0;
        for(auto i: it){
            REQUIRE(expected[count] == i);
            count++;
        }
        REQUIRE(5 == count);
    }

    SECTION("advances by a step size")
    {
        {
            std::vector<int> expected{5,8,11,14,17};
            auto it = range(5,20,3);
            auto count = 0;
            for(auto i: it){
                REQUIRE(expected[count] == i);
                count++;
            }
            REQUIRE(5 == count);
        }

        {
            std::vector<int> expected{5,8,11,14,17};
            auto it = range(5,19,3);
            auto count = 0;
            for(auto i: it){
                REQUIRE(expected[count] == i);
                count++;
            }
            REQUIRE(5 == count);
        }

        {
            std::vector<int> expected{5,8,11,14,17};
            auto it = range(5,18,3);
            auto count = 0;
            for(auto i: it){
                REQUIRE(expected[count] == i);
                count++;
            }
            REQUIRE(5 == count);
        }

        {
            std::vector<int> expected{10,6,2};
            auto it = range(10,0,-4);
            auto count = 0;
            for(auto i: it){
                REQUIRE(expected[count] == i);
                count++;
            }
            REQUIRE(3 == count);
        }

        {
            std::vector<int> expected{10,6,2};
            auto it = range(10,1,-4);
            auto count = 0;
            for(auto i: it){
                REQUIRE(expected[count] == i);
                count++;
            }
            REQUIRE(3 == count);
        }

        {
            std::vector<int> expected{10,6,2};
            auto it = range(10,-1,-4);
            auto count = 0;
            for(auto i: it){
                REQUIRE(expected[count] == i);
                count++;
            }
            REQUIRE(3 == count);
        }
    }

}


TEST_CASE("The map function")
{
    SECTION("applies a functor to all elements of an iterator")
    {
        std::vector<int> expected{0,2,4,6,8,10,12,14,16,18};
        auto twice = [](int x){return 2*x;};
        auto count = 0;
        for(auto i: map(twice,range(10))){
            REQUIRE(expected[count] == i);
            count++;
        }
        REQUIRE(10 == count);
    }

    SECTION("can take the result of the filter function as input")
    {
        std::vector<int> expected{2,4,6,8,10,12,14,16,18};
        auto twice = [](int x){return 2*x;};
        auto count = 0;
        for(auto i: map(twice,filter(range(10)))){
            REQUIRE(expected[count] == i);
            count++;
        }
        REQUIRE(9 == count);
    }
}

template<typename G,typename FN>
void iterate(G const& g, FN const& fn)
{
    for(auto const& x: g){ fn(x); }
}

TEST_CASE("The filter function")
{
    SECTION("returns an iterator for all elements a functor is true")
    {
        std::vector<int> expected{0,2,4,6,8};
        auto even = [](int x){return x%2 == 0;};
        auto count = 0;
        for(auto i: filter(even,range(10))){
            REQUIRE(expected[count] == i);
            count++;
        }
        REQUIRE(5 == count);
    }

    SECTION("defaults to a functor thats tests the truth value of the elements themselves")
    {
        auto even = [](int x){return x%2 == 0;};
        auto count = 0;
        for(auto i: filter(map(even,range(10)))){
            REQUIRE(i);
            count++;
        }
        REQUIRE(5 == count);
    }

    SECTION("can take an std vector as input")
    {
        std::vector<int> input{23,6,0,8,0,12,4,10,0};
        std::vector<int> expected{11,3,4,6,2,5};
        auto half = [](int x){return x/2;};
        auto count = 0;
        for(auto i: map(half,filter(input))){
            REQUIRE(expected[count] == i);
            count++;
        }
        REQUIRE(6 == count);
    }

    SECTION("can take an std vector as input and then be passed into map")
    {
        std::vector<int> input{23,6,0,8,0,12,4,10,0};
        auto half = [](int x){return x/2;};

        std::vector<int> v;
        iterate(map(half,filter(input)),[&](int x){
            v.push_back(x);
        });
        REQUIRE(6 == v.size());
    }
}




TEST_CASE("The zip function","allows parallel iteration over two iterators")
{
    std::vector<int> input{23,6,0,8,0,12,4,10,0};

    int count = 0;
    for(auto const& item : zip(input,range(input.size()*2))){
        REQUIRE(input.at(item.second) == item.first);
        count++;
    }
    REQUIRE(count == input.size());

    count = 0;
    for(auto const& item : zip(range(input.size()*2),input)){
        REQUIRE(input[item.first] == item.second);
        count++;
    }
    REQUIRE(count == input.size());
}

TEST_CASE("The enumerate function")
{
    std::vector<int> input{23,6,0,8,0,12,4,10,0};

    SECTION("allows iteration with an index")
    {

        int count = 0;
        for(auto const& i : enumerate(input)){
            REQUIRE(input[i.nr] == i.item);
            count++;
        }
        REQUIRE(count == input.size());
    }

    SECTION("allows changing the elements")
    {
        for(auto i : enumerate(input)){
            i.item = i.nr*2;
        }

        for(auto const& i : enumerate(input)){
            REQUIRE((i.nr*2) == i.item);
        }
    }
}


TEST_CASE("The reduce function","does reduction")
{
    std::vector<int> input{23,6,0,8,0,12,4,10,0};

    auto sum = reduce(input,0,[](int a, int b){ return a+b; });
    REQUIRE(63 == sum);
}


TEST_CASE("The cycle function", "makes an infinite iterator")
{
    std::vector<int> expected{0,1,2,0,1,2,0,1,2,0,1,2};
    int i = 0;
    for(auto const& item : cycle(range(3))){
        CHECK(expected[i] == item);
        i++;
        if(i > 10){
            break;
        }
    }
    CHECK(11 == i);
}


