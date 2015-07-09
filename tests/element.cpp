#include <cstdio>
#include <fn/element.hpp>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"
using namespace fn;


TEST_CASE("The element function")
{
    SECTION("accesses_containers_of_optionals")
    {
        std::vector<optional<int>> v{11,{}};
        v.push_back(4);
        v.push_back(6);
        v.push_back(2);
        v.push_back(5);

        use_(~element(3).of(v))_as(i){
            REQUIRE(6 == i);
        }
        ||[]{ CHECK(false); };

        use_(~element(1).of(v))_as(i){
            (void)i;
            CHECK(false);
        };

        REQUIRE(11 ==  ~~element(0).of(v));
        REQUIRE(0 ==  ~~element(1).of(v));

        std::map<std::string,optional<int>> m = {{
            {"one",1},
            {"two",2},
            {"none",{}},
        }};

        use_(~element("one").in(m))_as(i){
            REQUIRE(1 == i);
        }
        ||[]{ CHECK(false); };

        ~element("one").in(m)
        >>[](int i){
            REQUIRE(1 == i);
        }
        ||[]{
            CHECK(false);
        };

        use_(~element("none").in(m))_as(i){
            (void)i;
            CHECK(false);
        };
    }

    SECTION("creates_iterator_to_ranges")
    {
        std::vector<int> v{11,3,4,6,2,5};

        {
            std::vector<int> out;
            for(auto i: element(0).to_last().of(v)){
                out.push_back(i);
            }
            REQUIRE(v == out);
        }

        {
            std::vector<int> out;
            for(auto i: element(0).to(0).of(v)){
                out.push_back(i);
            }
            REQUIRE(v == out);
        }

        {
            std::vector<int> out;
            for(auto i: element(0).to(0).of(v)){
                out.push_back(i);
            }
            REQUIRE(v == out);
        }

        {
            std::vector<int> e{3,4,6,2,5};
            std::vector<int> out;
            for(auto i: element(1).to(0).of(v)){
                out.push_back(i);
            }
            REQUIRE(e == out);
        }

        {
            std::vector<int> e{3,4,6,2};
            std::vector<int> out;
            for(auto i: element(1).to(-1).of(v)){
                out.push_back(i);
            }
            REQUIRE(e == out);
        }

        {
            std::vector<int> e{3,4,6,2};
            std::vector<int> out;
            for(auto i: element(1).to(-1).of(v)){
                out.push_back(i);
            }
            REQUIRE(e == out);
        }

        {
            std::vector<int> e{3,4,6,2};
            std::vector<int> out;
            for(auto i: element(-5).to(5).of(v)){
                out.push_back(i);
            }
            REQUIRE(e == out);
        }

        {
            std::vector<int> e{11,3,4,6,2};
            std::vector<int> out;
            for(auto i: element(-55).to(5).of(v)){
                out.push_back(i);
            }
            REQUIRE(e == out);
        }

        {
            std::vector<int> e{11,3,4,6,2,5};
            std::vector<int> out;
            for(auto i: element(-55).to(500).of(v)){
                out.push_back(i);
            }
            REQUIRE(e == out);
        }

        {
            std::vector<int> e{};
            std::vector<int> out;
            for(auto i: element(4).to(3).of(v)){
                out.push_back(i);
            }
            REQUIRE(e == out);
        }

        {
            std::vector<int> e{11,4,2};
            std::vector<int> out;
            for(auto i: element(0).to_last().of(v).by(2)){
                out.push_back(i);
            }
            REQUIRE(e == out);
        }
    }

}
