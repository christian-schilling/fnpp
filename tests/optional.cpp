#include <cstdio>
#include <fn/optional.hpp>
#include <fn/iterators.hpp>
#include <fn/element.hpp>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"
using namespace fn;

optional<int> twice_in_range(int from, int to, int i)
{
    if(i>to){
        return {};
    }
    if(i<from){
        return {};
    }
    return {i*2};
}

inline extern size_t f(size_t i)
{
    if(i!=1){
        return i*f(i-1);
    }
    return 1;
}

std::vector<double> f1(int i){
    return {double(i)};
}

std::vector<double> f2(std::vector<double> v){
    return v;
}

std::vector<double> f3()
{
    return {};
}

optional<std::string> maybe_hello(int i)
{
    if(i==1){
        return std::string("hello");
    }
    else{
        return {};
    }
}

struct OnlyMove
{
    static int constructed;
    static int destructed;
    static int moved;
    int i;

    OnlyMove(): i{111}
    {
        printf("create\n");
        constructed++;
    }

    ~OnlyMove()
    {
        printf("destroy %d\n",i);
        destructed++;
    }

    OnlyMove(OnlyMove const& o) = delete;

    OnlyMove(OnlyMove&& o): i{o.i}
    {
        moved++;
        o.i = 999;
        printf("move\n");
    }

    OnlyMove& operator=(OnlyMove&& o)
    {
        moved++;
        i = o.i;
        o.i = 999;
        printf("move\n");
        return *this;
    }
};

int OnlyMove::constructed = 0;
int OnlyMove::destructed = 0;
int OnlyMove::moved = 0;

struct NonTrivial
{
    static int constructed;
    static int destructed;
    int i;

    NonTrivial(int const& i): i(i) {
        constructed++;
    }

    ~NonTrivial()
    {
        destructed++;
    }

    NonTrivial(NonTrivial const& o): i(o.i)
    {
        printf("copy\n");
    }
};

int NonTrivial::constructed = 0;
int NonTrivial::destructed = 0;

optional<NonTrivial> maybe_nt(int i)
{
    if(i==1){
        return {i};
    }
    else{
        return {};
    }
}

void take(optional<int> const& i)
{
    REQUIRE(12 == (i | 0));
}

optional<int> getnr(int nr)
{
    if(nr % 2){
        return {};
    }
    else{
        return nr;
    }
}

TEST_CASE("An optional value")
{
    SECTION(" can_be_defaulted_to_a_constant")
    {
        std::vector<int> in{1,4,6,2,10,55};
        std::vector<int> expected{-1,8,12,-1,-1,-1};
        for(size_t i=0;i<in.size();i++){
            auto r = twice_in_range(3,7,in[i]) | -1;
            REQUIRE(expected[i] == r);
        }

        std::string const bla{"bla"};
        optional<std::string const> i{bla};
        REQUIRE("bla" == ~i);
    }

    SECTION(" can_be_defaulted_to_a_reference")
    {
        int i = 0;
        int j = 0;
        optional<int&> oi{i};

        (oi | j) = 1;

        REQUIRE(1 == i);
        REQUIRE(0 == j);
        REQUIRE(1 == (oi | 2));

        optional<int&> ni;

        (ni | j) = 2;

        REQUIRE(1 == i);
        REQUIRE(2 == j);
        REQUIRE(3 == (ni | 3));
    }


    SECTION(" can_change_if_it_is_a_reference")
    {
        int i = 5;
        int j = 7;
        optional<int&> o = i;

        auto& r = o | j;

        REQUIRE(5 == r);
        r = 6;
        REQUIRE(6 == i);

        with_(o){ o = 9; };

        REQUIRE(9 == i);
    }

    SECTION("can be const itself")
    {
        int i = 3;
        optional<int&> const o{i};
        REQUIRE(3 == (o | 0));

        o >>[](int i){
            REQUIRE(3 == i);
        };
    }

    SECTION("supports handler chaining")
    {
        optional<int> i = 4;

        auto const x = i
        >>[](int v){ return v*2; }
        >>[](int v){ return v*3; }
        >>[](int v){ return v*5; }
        ||[]{ return 1111; };

        REQUIRE((4*2*3*5) ==  x);

        auto const y = i
        >>[](int v){ return v*2; }
        >>[](int  )->optional<int>{ return {}; }
        >>[](int v){ return v*3; }
        | 1111;

        REQUIRE(1111 ==  y);
    }

    SECTION(" chained_handlers_can_convert_types")
    {
        optional<int> i = 4;

        {
            auto w = i
                >>[](int i)->std::vector<double>{
                    return std::vector<double>{double(i)};
                }
            >>[](std::vector<double> v)->optional<std::vector<double>>{
                return v;
            }
            ||[]{
                return std::vector<double>{};
            };


            auto v = w;

            REQUIRE(1 == v.size());
            REQUIRE(4 == v.front());
        }

        {
            auto w = i >> f1 || f3;
            auto v = w;

            REQUIRE(1 == v.size());
            REQUIRE(4 == v.front());
        }

        {
            auto w = i >> f1 >> f2 || f3;
            auto v = w;

            REQUIRE(1 == v.size());
            REQUIRE(4 == v.front());
        }
    }


    SECTION(" Move_Semantics")
    {
        OnlyMove::constructed = 0;
        OnlyMove::destructed = 0;
        OnlyMove::moved = 0;

        auto v = new optional<OnlyMove>{OnlyMove{}};

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(1 == OnlyMove::destructed);
        REQUIRE(1 == OnlyMove::moved);

        *v >>[](OnlyMove& om){om.i = 123;};

        REQUIRE(123 == (*v >>[](OnlyMove& om){return om.i;} ||[]{return -1;}));

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(1 == OnlyMove::destructed);
        REQUIRE(1 == OnlyMove::moved);

        auto v2 = new optional<OnlyMove>{std::move(*v)};

        REQUIRE_FALSE(v->valid());
        REQUIRE(v2->valid());

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(2 == OnlyMove::destructed);
        REQUIRE(2 == OnlyMove::moved);

        REQUIRE(123 == (*v2 >>[](OnlyMove& om){return om.i;} ||[]{return -1;}));

        *v2 = std::move(*v);

        REQUIRE_FALSE(v->valid());
        REQUIRE_FALSE(v2->valid());

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(3 == OnlyMove::destructed);
        REQUIRE(2 == OnlyMove::moved);

        delete v;
        delete v2;

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(3 == OnlyMove::destructed);
        REQUIRE(2 == OnlyMove::moved);
    }


    SECTION(" Move_Semantics_const")
    {
        OnlyMove::constructed = 0;
        OnlyMove::destructed = 0;
        OnlyMove::moved = 0;

        auto v = new optional<OnlyMove>{OnlyMove{}};

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(1 == OnlyMove::destructed);
        REQUIRE(1 == OnlyMove::moved);

        *v >>[](OnlyMove& om){om.i = 123;};

        REQUIRE(123 == (*v >>[](OnlyMove& om){return om.i;} ||[]{return -1;}));

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(1 == OnlyMove::destructed);
        REQUIRE(1 == OnlyMove::moved);

        auto v2 = new optional<OnlyMove const>{std::move(*v)};

        REQUIRE_FALSE(v->valid());
        REQUIRE(v2->valid());

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(2 == OnlyMove::destructed);
        REQUIRE(2 == OnlyMove::moved);

        REQUIRE(123 == (*v2 >>[](OnlyMove const& om){return om.i;} ||[]{return -1;}));

        delete v;
        delete v2;

        REQUIRE(1 == OnlyMove::constructed);
        REQUIRE(3 == OnlyMove::destructed);
        REQUIRE(2 == OnlyMove::moved);
    }

    SECTION("can_be_filled_by_assignment")
    {
        optional<int>i;

        REQUIRE_FALSE(i.valid());
        i = 5;
        REQUIRE(i.valid());
        REQUIRE(5 == ~i);

        int j = 7;
        i = {};

        REQUIRE_FALSE(i.valid());
        i = j;
        REQUIRE(i.valid());
        REQUIRE(7 == ~i);

        optional<int> k = 8;
        i = {};

        REQUIRE_FALSE(i.valid());
        i = k;
        REQUIRE(i.valid());
        REQUIRE(8 == ~i);

        REQUIRE(8 == ~k);
    }



    SECTION(" works_with_nontrivial_types")
    {
        for(auto const i: range(5)){
            auto s = use_(maybe_hello(i))_as(s){
                return s;
            }
            ||[]{
                return std::string("nothing");
            };

            if(i==1){
                REQUIRE("hello" == s);
            }
            else{
                REQUIRE("nothing" == s);
            }
        }

        std::vector<std::string> v;

        auto s = [=]()->optional<std::string>{
            return element(0).of(v);
        }();
        REQUIRE("nothing" == (s | "nothing"));

        REQUIRE(0 == NonTrivial::constructed);
        maybe_nt(0);
        REQUIRE(0 == NonTrivial::constructed);
        maybe_nt(1);
        REQUIRE(1 == NonTrivial::constructed);
    }

    SECTION("Optional_handlers"," can_return_a_reference")
    {
        static double d = 34;
        int i = 9;
        optional<int&> o{i};

        auto& dr = o
        >>[&](int&)->double&{ return d; }
        ||[&]()->double&{ return d; };

        CHECK(Approx(34) == dr);
        d = 100;
        CHECK(Approx(100) == dr);
    }


    SECTION("7optional"," can_be_cleared")
    {
        optional<int> i = 5;
        REQUIRE(5 == ~i);

        i = {};
        REQUIRE(0 == ~i);

        NonTrivial::constructed = 0;
        NonTrivial::destructed = 0;

        optional<NonTrivial> nt {5};

        REQUIRE(1 == NonTrivial::constructed);
        REQUIRE(1 == NonTrivial::destructed);

        nt = {};

        REQUIRE(1 == NonTrivial::constructed);
        REQUIRE(2 == NonTrivial::destructed);

        nt = {};

        REQUIRE(1 == NonTrivial::constructed);
        REQUIRE(2 == NonTrivial::destructed);
    }



    SECTION("Value_optionals"," call_the_destructor_when_destroyed")
    {
        NonTrivial::constructed = 0;
        NonTrivial::destructed = 0;
        REQUIRE(0 == NonTrivial::constructed);
        REQUIRE(0 == NonTrivial::destructed);
        auto vo = new optional<NonTrivial>{5};
        REQUIRE(1 == NonTrivial::constructed);
        REQUIRE(1 == NonTrivial::destructed);
        delete vo;
        REQUIRE(1 == NonTrivial::constructed);
        REQUIRE(2 == NonTrivial::destructed);
    }

    SECTION(" can_call_a_handler_depending_on_its_status")
    {
        std::vector<int> in{1,4,6,2,10,55};
        std::vector<int> expected{0,8*3,12*3,0,0,0};
        for(size_t i=0;i<in.size();i++){
            int out = 0;
            optional<int> result = twice_in_range(3,7,in[i]);
            result >> [&](int v){out=v*3;};
            REQUIRE(expected[i] == out);
        }
    }


    SECTION(" can_be_used_by_providing_handlers_for_both_cases")
    {
        std::vector<int> in{1,4,6,2,10,55};
        std::vector<int> expected{-1,16,24,-1,-1,-1};
        for(size_t i=0;i<in.size();i++){
            auto result = twice_in_range(3,7,in[i]);
            auto r = result
                >>[](int v){return 2 * v; }
                ||[]{return -1;};

            REQUIRE(expected[i] == r);
        }
    }

    SECTION(" can_be_tested_if_it_has_a_value")
    {
        auto t = optional<int>{1};
        REQUIRE(t.valid());

        auto t0 = optional<int>{0};
        REQUIRE(t0.valid());

        auto f = optional<int>{};
        REQUIRE_FALSE(f.valid());
    }

    SECTION(" can_be_passed_into_a_function")
    {
        auto r = twice_in_range(3,9,6);
        take(r);
        REQUIRE(12 == (r | 0));
    }


    SECTION("Two_optional_values"," can_tested_for_equality")
    {
        {
            optional<int> a = 4;
            optional<int> b = 3;
            CHECK(a != b);
            CHECK(a != 3);
        }

        {
            optional<int> a = 4;
            optional<int> b = 4;
            REQUIRE(a == b);
            REQUIRE(a == 4);
        }

        {
            optional<int> a = {};
            optional<int> b = 4;
            CHECK(a != b);
            CHECK(a != 4);
            CHECK(a != 0);
        }

        {
            optional<int> a = {};
            optional<int> b = {};
            CHECK(a != b);
        }
    }

    SECTION("A_non_const_optional"," can_be_converted_to_a_const_one")
    {
        optional<int> a{100};
        optional<int const> b{a};

        REQUIRE(a.valid());
        REQUIRE(100 ==  (a | 1));

        REQUIRE(b.valid());
        REQUIRE(100 ==  (b | 1));
    }

    SECTION("A_const_optional"," can_be_converted_to_a_non_const_one")
    {
        optional<int const> a{100};
        optional<int> b{a};

        REQUIRE(a.valid());
        REQUIRE(100 ==  (a | 1));

        REQUIRE(b.valid());
        REQUIRE(100 ==  (b | 1));
    }

    SECTION("4A_non_const_optional"," can_be_converted_to_a_optional_const_reference")
    {
        optional<int> a{100};
        optional<int const&> cr{a};

        REQUIRE(a.valid());
        REQUIRE(100 ==  (a | 1));

        REQUIRE(cr.valid());
        REQUIRE(100 ==  (cr | 1));

        a >>[](int& v) {v = 1000;};
        REQUIRE(cr.valid());
        REQUIRE(1000 ==  (cr | 1));
    }

    SECTION("3A_non_const_optional"," can_be_converted_to_a_optional_reference")
    {
        optional<int> a{100};
        optional<int&> r{a};
        optional<int const&> cr{r};

        REQUIRE(a.valid());
        REQUIRE(100 ==  (a | 1));

        REQUIRE(cr.valid());
        REQUIRE(100 ==  (cr | 1));

        REQUIRE(r.valid());
        REQUIRE(100 ==  (r | 1));

        a >>[](int& v) {v = 1000;};
        REQUIRE(r.valid());
        REQUIRE(1000 ==  (r | 1));
        REQUIRE(1000 ==  (a | 1));
        REQUIRE(1000 ==  (cr | 1));

        optional<int> n{r};
        optional<int> c{cr};
        optional<int const> cc{a};
        optional<int const> ccr{r};
        a >>[](int& v) {v = 500;};

        REQUIRE(1000 ==  (n | 1));
        REQUIRE(1000 ==  (c | 1));
        REQUIRE(1000 ==  (cc | 1));
        REQUIRE(1000 ==  (ccr | 1));
    }


    SECTION(" can_be_accessed_easier_using_macros")
    {
        for(auto const i: range(5)){
            int out;
            auto nr = getnr(i);

            out = -2;
            with_(nr){
                out = nr;
            };
            if(!(i%2)){ REQUIRE(i == out); }
            else{ REQUIRE(-2 == out); }

            out = -2;
            nr ||[&]{
                out = -1;
            };
            if(!(i%2)){ REQUIRE(-2 == out); }
            else{ REQUIRE(-1 == out); }

            out = -2;
            use_(getnr(i))_as(nr){
                out = nr;
            };
            if(!(i%2)){ REQUIRE(i == out); }
            else{ REQUIRE(-2 == out); }

            out = -2;
            use_(getnr(i))_as(nr){
                out = nr;
            }
            ||[&]{
                out = -1;
            };
            if(!(i%2)){ REQUIRE(i == out); }
            else{ REQUIRE(-1 == out); }
        }
    }

    SECTION("The_macros"," work_also_with_references_to_optionals")
    {
        for(auto const i: range(5)){
            int out;
            auto x = getnr(i);
            auto& nr = x;

            out = -2;
            with_(nr){
                out = nr;
            };
            if(!(i%2)){ REQUIRE(i == out); }
            else{ REQUIRE(-2 == out); }

            out = -2;
            nr ||[&]{
                out = -1;
            };
            if(!(i%2)){ REQUIRE(-2 == out); }
            else{ REQUIRE(-1 == out); }

            out = -2;
            use_(nr)_as(nr){
                out = nr;
            };
            if(!(i%2)){ REQUIRE(i == out); }
            else{ REQUIRE(-2 == out); }

            out = use_(nr)_as(nr){
                return nr;
            }
            ||[]{
                return -1;
            };
            if(!(i%2)){ REQUIRE(i == out); }
            else{ REQUIRE(-1 == out); }
        }
    }

    SECTION("as_range","makes_begin_end_pairs_compatible_with_range_based_for")
    {
        std::vector<int> in{1,4,6,2,10,55};
        size_t n = 0;

        for(auto const i: as_range(in.begin(),in.end())){
            REQUIRE(in[n] == i);
            n++;
        }

        REQUIRE(in.size() == n);
    }

    SECTION("The_element_function"," accesses_containers_with_range_checking_returning_an_optional")
    {
        std::vector<int> v{11,3,4,6,2,5};

        {
            auto t = element(3).of(v);
            REQUIRE(t.valid());
            REQUIRE(6 ==  (t | 99));
            REQUIRE(6 ==  ~t);
        }

        {
            auto t = element(30).of(v);
            REQUIRE_FALSE(t.valid());
            REQUIRE(99 ==  (t | 99));
            REQUIRE(0 ==  ~t);
        }

        {
            auto second = element(1);
            auto t = second.of(v);
            REQUIRE(t.valid());
            REQUIRE(3 ==  (t | 99));
        }

        {
            auto t = element(-1).of(v);
            REQUIRE(t.valid());
            REQUIRE(5 ==  (t | 99));
        }

        {
            std::vector<int> w;
            REQUIRE_FALSE(element(-1).of(w).valid());
            REQUIRE_FALSE(element(0).of(w).valid());
        }

        {
            REQUIRE(6 ==  v.at(3));
            use_(element(3).of(v))_as(i){
                i = 4;
            }
            ||[]{ CHECK(false); };
            REQUIRE(4 ==  v.at(3));
        }

        std::map<std::string,int> m = {{
            {"one",1},
            {"two",2},
        }};

        {
            auto o = element("one").in(m);
            REQUIRE(o.valid());
            REQUIRE(1 ==  (o | 99));
        }

        {
            auto o = element("not").in(m);
            REQUIRE_FALSE(o.valid());
        }

        {
            auto two = element("two");
            auto o = two.in(m);
            REQUIRE(o.valid());
            REQUIRE(2 ==  (o | 99));
        }
    }

    SECTION(" supports_assignment")
    {
        int a = 0;
        int b = 5;

        std::vector<optional<int&>> v{ a, b };

        use_(element(1).of(v))_as(i){
            REQUIRE(5 ==  ~i);
            b = 6;
            REQUIRE(6 ==  ~i);
        };

        v[1] = a;

        use_(element(1).of(v))_as(i){
            REQUIRE(0 ==  ~i);
            a = 13;
            REQUIRE(13 ==  ~i);
        };

        std::vector<optional<int>> w{ 2, 3, 4, 5 };

        REQUIRE(3 ==  ~~element(1).of(w));
        w[1] = 50;
        REQUIRE(50 ==  ~~element(1).of(w));

        w[1] = w[3];
        REQUIRE(5 ==  ~~element(1).of(w));

        w[3] >>[](int& i){i = 99;};
        REQUIRE(99 ==  ~~element(3).of(w));
        REQUIRE(5 ==  ~~element(1).of(w));
    }
}
