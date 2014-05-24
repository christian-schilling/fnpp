#include "fn++.hpp"
#include <stdio.h>
#include <vector>
#include <map>
#include <utility>
#include <gtest/gtest.h>

using namespace fn;

TEST(A_range,
goes_from_zero_to_the_upper_limit_not_including_the_limit)
{
    std::vector<int> expected{0,1,2,3,4,5,6,7,8,9};
    auto it = range(10);
    auto count = 0;
    for(auto i: it){
        EXPECT_EQ(expected[count],i);
        count++;
    }
    EXPECT_EQ(10,count);
}

TEST(The_map_function,
applies_a_functor_to_all_elements_of_an_iterator)
{
    std::vector<int> expected{0,2,4,6,8,10,12,14,16,18};
    auto twice = [](int x){return 2*x;};
    auto count = 0;
    for(auto i: map(twice,range(10))){
        EXPECT_EQ(expected[count],i) << i;
        count++;
    }
    EXPECT_EQ(10,count);
}

TEST(The_filter_function,
returns_an_iterator_for_all_elements_a_functor_is_true)
{
    std::vector<int> expected{0,2,4,6,8};
    auto even = [](int x){return x%2 == 0;};
    auto count = 0;
    for(auto i: filter(even,range(10))){
        EXPECT_EQ(expected[count],i) << i;
        count++;
    }
    EXPECT_EQ(5,count);
}

TEST(The_filter_function,
defaults_to_a_functor_thats_tests_the_truth_value_of_the_elements_themselves)
{
    std::vector<int> expected{1,1,1,1,1};
    auto even = [](int x){return x%2 == 0;};
    auto count = 0;
    for(auto i: filter(map(even,range(10)))){
        EXPECT_EQ(expected[count],i) << i;
        count++;
    }
    EXPECT_EQ(5,count);
}

TEST(The_map_function,
can_take_the_result_of_the_filter_function_as_input)
{
    std::vector<int> expected{2,4,6,8,10,12,14,16,18};
    auto twice = [](int x){return 2*x;};
    auto count = 0;
    for(auto i: map(twice,filter(range(10)))){
        EXPECT_EQ(expected[count],i) << i;
        count++;
    }
    EXPECT_EQ(9,count);
}

TEST(The_filter_function,
can_take_an_std_vector_as_input)
{
    std::vector<int> input{23,6,0,8,0,12,4,10,0};
    std::vector<int> expected{11,3,4,6,2,5};
    auto half = [](int x){return x/2;};
    auto count = 0;
    for(auto i: map(half,filter(input))){
        EXPECT_EQ(expected[count],i) << i;
        count++;
    }
    EXPECT_EQ(6,count);
}

template<typename G,typename FN>
void iterate(G const& g, FN const& fn)
{
    for(auto const& x: g){ fn(x); }
}

TEST(The_filter_function,
can_take_an_std_vector_as_input_and_then_be_passed_into_map)
{
    std::vector<int> input{23,6,0,8,0,12,4,10,0};
    auto half = [](int x){return x/2;};

    std::vector<int> v;
    iterate(map(half,filter(input)),[&](int x){
        v.push_back(x);
    });
    EXPECT_EQ(6,v.size());
}


TEST(The_zip_function,
allows_parallel_iteration_over_two_iterators)
{
    std::vector<int> input{23,6,0,8,0,12,4,10,0};

    int count = 0;
    for(auto const& item : zip(input,range(input.size()*2))){
        EXPECT_EQ(input[item.second],item.first);
        count++;
    }
    EXPECT_EQ(count,input.size());

    count = 0;
    for(auto const& item : zip(range(input.size()*2),input)){
        EXPECT_EQ(input[item.first],item.second);
        count++;
    }
    EXPECT_EQ(count,input.size());
}

TEST(The_enumerate_function,
allows_iteration_with_an_index)
{
    std::vector<int> input{23,6,0,8,0,12,4,10,0};

    int count = 0;
    for(auto const& i : enumerate(input)){
        EXPECT_EQ(input[i.nr],i.item);
        count++;
    }
    EXPECT_EQ(count,input.size());
}

TEST(The_reduce_function,
does_reduction)
{
    std::vector<int> input{23,6,0,8,0,12,4,10,0};

    auto sum = reduce(input,0,[](int a, int b){ return a+b; });
    EXPECT_EQ(63,sum);
}

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

TEST(An_optional_value,
can_be_defaulted_to_a_constant)
{
    std::vector<int> in{1,4,6,2,10,55};
    std::vector<int> expected{-1,8,12,-1,-1,-1};
    for(size_t i=0;i<in.size();i++){
        auto r = twice_in_range(3,7,in[i]) or -1;
        EXPECT_EQ(expected[i],r);
    }
}

TEST(An_optional_value,
can_be_defaulted_to_a_reference)
{
    int i = 0;
    int j = 0;
    optional<int&> oi{i};

    (oi or j) = 1;

    EXPECT_EQ(1,i);
    EXPECT_EQ(0,j);
    EXPECT_EQ(1,oi or 2);

    optional<int&> ni;

    (ni or j) = 2;

    EXPECT_EQ(1,i);
    EXPECT_EQ(2,j);
    EXPECT_EQ(3,ni or 3);

    optional<int const&> ci{4};

    EXPECT_EQ(4,ci or 3);
}


TEST(An_optional_value,
can_change_if_it_is_a_reference)
{
    int i = 5;
    int j = 7;
    optional<int&> o = i;

    auto& r = o or j;

    EXPECT_EQ(5,r);
    r = 6;
    EXPECT_EQ(6,i);

    with_(o){ o = 9; };

    EXPECT_EQ(9,i);
}

TEST(An_optional_value,
can_be_const_itself)
{
    int i = 3;
    optional<int&> const o{i};
    EXPECT_EQ(3,o or 0);

    o >>[](int i){
        EXPECT_EQ(3,i);
    };
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

struct NonTrivial
{
    static int constructed;
    int i;
    NonTrivial(int i): i(i) {
        constructed++;
    }
};

int NonTrivial::constructed = 0;

optional<NonTrivial> maybe_nt(int i)
{
    if(i==1){
        return {i};
    }
    else{
        return {};
    }
}


TEST(An_optional_value,
works_with_nontrivial_types)
{
    for(auto const i: range(5)){
        auto s = use_(maybe_hello(i))_as(s){
            return s;
        }
        >>[]{
            return std::string("nothing");
        };

        if(i==1){
            EXPECT_EQ("hello",s);
        }
        else{
            EXPECT_EQ("nothing",s);
        }
    }

    std::vector<std::string> v;

    auto s = [=]()->optional<std::string>{
        return element(0).of(v);
    }();
    EXPECT_EQ("nothing",s or "nothing");

    EXPECT_EQ(0,NonTrivial::constructed);
    maybe_nt(0);
    EXPECT_EQ(0,NonTrivial::constructed);
    maybe_nt(1);
    EXPECT_EQ(1,NonTrivial::constructed);
}

TEST(An_optional_value,
can_call_a_handler_depending_on_its_status)
{
    std::vector<int> in{1,4,6,2,10,55};
    std::vector<int> expected{0,8*3,12*3,0,0,0};
    for(size_t i=0;i<in.size();i++){
        int out = 0;
        optional<int> result = twice_in_range(3,7,in[i]);
        result >> [&](int v){out=v*3;};
        EXPECT_EQ(expected[i],out);
    }
}


TEST(An_optional_value,
can_be_used_by_providing_handlers_for_both_cases)
{
    std::vector<int> in{1,4,6,2,10,55};
    std::vector<int> expected{-1,16,24,-1,-1,-1};
    for(size_t i=0;i<in.size();i++){
        auto result = twice_in_range(3,7,in[i]);
        auto r = result
            >>[](int v){return 2 * v; }
            >>[]{return -1;};

        EXPECT_EQ(expected[i],r);
    }
}

TEST(An_optional_value,
can_be_tested_if_it_has_a_value)
{
    auto t = optional<int>{1};
    EXPECT_TRUE(t.has_value);

    auto t0 = optional<int>{0};
    EXPECT_TRUE(t0.has_value);

    auto f = optional<int>{};
    EXPECT_FALSE(f.has_value);
}

void take(optional<int> const& i)
{
    EXPECT_EQ(12,i or 0);
}

TEST(An_optional_value,
can_be_passed_into_a_function)
{
    auto r = twice_in_range(3,9,6);
    take(r);
    EXPECT_EQ(12,r or 0);
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

TEST(Two_optional_values,
can_tested_for_equality)
{
    {
        optional<int> a = 4;
        optional<int> b = 3;
        EXPECT_NE(a,b);
        EXPECT_NE(a,3);
    }

    {
        optional<int> a = 4;
        optional<int> b = 4;
        EXPECT_EQ(a,b);
        EXPECT_EQ(a,4);
    }

    {
        optional<int> a = {};
        optional<int> b = 4;
        EXPECT_NE(a,b);
        EXPECT_NE(a,4);
        EXPECT_NE(a,0);
    }

    {
        optional<int> a = {};
        optional<int> b = {};
        EXPECT_NE(a,b);
    }
}

TEST(A_non_const_optional,
can_be_converted_to_a_const_one)
{
    optional<int> a{100};
    optional<int const> b{a};

    EXPECT_TRUE(a.has_value);
    EXPECT_EQ(100, a or 1);

    EXPECT_TRUE(b.has_value);
    EXPECT_EQ(100, b or 1);
}

TEST(A_const_optional,
can_be_converted_to_a_non_const_one)
{
    optional<int const> a{100};
    optional<int> b{a};

    EXPECT_TRUE(a.has_value);
    EXPECT_EQ(100, a or 1);

    EXPECT_TRUE(b.has_value);
    EXPECT_EQ(100, b or 1);
}

TEST(A_non_const_optional,
can_be_converted_to_a_optional_const_reference)
{
    optional<int> a{100};
    optional<int const&> cr{a};

    EXPECT_TRUE(a.has_value);
    EXPECT_EQ(100, a or 1);

    EXPECT_TRUE(cr.has_value);
    EXPECT_EQ(100, cr or 1);

    a >>[](int& v) {v = 1000;};
    EXPECT_TRUE(cr.has_value);
    EXPECT_EQ(1000, cr or 1);
}

TEST(A_non_const_optional,
can_be_converted_to_a_optional_reference)
{
    optional<int> a{100};
    optional<int&> r{a};
    optional<int const&> cr{r};

    EXPECT_TRUE(a.has_value);
    EXPECT_EQ(100, a or 1);

    EXPECT_TRUE(cr.has_value);
    EXPECT_EQ(100, cr or 1);

    EXPECT_TRUE(r.has_value);
    EXPECT_EQ(100, r or 1);

    a >>[](int& v) {v = 1000;};
    EXPECT_TRUE(r.has_value);
    EXPECT_EQ(1000, r or 1);
    EXPECT_EQ(1000, a or 1);
    EXPECT_EQ(1000, cr or 1);

    optional<int> n{r};
    EXPECT_EQ(1000, n or 1);

    optional<int> c{cr};
    EXPECT_EQ(1000, c or 1);
}


TEST(An_optional_value,
can_be_accessed_easier_using_macros)
{
    for(auto const i: range(5)){
        int out;
        auto nr = getnr(i);

        out = -2;
        with_(nr){
            out = nr;
        };
        if(!(i%2)){ EXPECT_EQ(i,out); }
        else{ EXPECT_EQ(-2,out); }

        out = -2;
        !nr >>[&]{
            out = -1;
        };
        if(!(i%2)){ EXPECT_EQ(-2,out); }
        else{ EXPECT_EQ(-1,out); }

        out = -2;
        use_(getnr(i))_as(nr){
            out = nr;
        };
        if(!(i%2)){ EXPECT_EQ(i,out) << i; }
        else{ EXPECT_EQ(-2,out) << i; }

        out = -2;
        use_(getnr(i))_as(nr){
            out = nr;
        }
        >>[&]{
            out = -1;
        };
        if(!(i%2)){ EXPECT_EQ(i,out) << i; }
        else{ EXPECT_EQ(-1,out) << i; }
    }
}

TEST(The_macros,
work_also_with_references_to_optionals)
{
    for(auto const i: range(5)){
        int out;
        auto x = getnr(i);
        auto& nr = x;

        out = -2;
        with_(nr){
            out = nr;
        };
        if(!(i%2)){ EXPECT_EQ(i,out); }
        else{ EXPECT_EQ(-2,out); }

        out = -2;
        !nr >>[&]{
            out = -1;
        };
        if(!(i%2)){ EXPECT_EQ(-2,out); }
        else{ EXPECT_EQ(-1,out); }

        out = -2;
        use_(nr)_as(nr){
            out = nr;
        };
        if(!(i%2)){ EXPECT_EQ(i,out) << i; }
        else{ EXPECT_EQ(-2,out) << i; }

        out = use_(nr)_as(nr){
            return nr;
        }
        >>[]{
            return -1;
        };
        if(!(i%2)){ EXPECT_EQ(i,out) << i; }
        else{ EXPECT_EQ(-1,out) << i; }
    }
}

TEST(as_range,makes_begin_end_pairs_compatible_with_range_based_for)
{
    std::vector<int> in{1,4,6,2,10,55};
    size_t n = 0;

    for(auto const i: as_range(in.begin(),in.end())){
        EXPECT_EQ(in[n],i);
        n++;
    }

    EXPECT_EQ(in.size(),n);
}

TEST(The_element_function,
accesses_containers_with_range_checking_returning_an_optional)
{
    std::vector<int> v{11,3,4,6,2,5};
    int n = 0;

    {
        auto t = element(3).of(v);
        EXPECT_TRUE(t.has_value);
        EXPECT_EQ(6, t or n);
    }

    {
        auto t = element(30).of(v);
        EXPECT_FALSE(t.has_value);
        EXPECT_EQ(0, t or n);
    }

    {
        auto second = element(1);
        auto t = second.of(v);
        EXPECT_TRUE(t.has_value);
        EXPECT_EQ(3, t or n);
    }

    {
        auto t = element(-1).of(v);
        EXPECT_TRUE(t.has_value);
        EXPECT_EQ(5, t or n);
    }

    {
        std::vector<int> w;
        EXPECT_FALSE(element(-1).of(w).has_value);
        EXPECT_FALSE(element(0).of(w).has_value);
    }

    {
        EXPECT_EQ(6, v.at(3));
        use_(element(3).of(v))_as(i){
            i = 4;
        }
        >>[]{ ADD_FAILURE(); };
        EXPECT_EQ(4, v.at(3));
    }

    std::map<std::string,int> m = {{
        {"one",1},
        {"two",2},
    }};

    {
        auto o = element("one").in(m);
        EXPECT_TRUE(o.has_value);
        EXPECT_EQ(1, o or n);
    }

    {
        auto o = element("not").in(m);
        EXPECT_FALSE(o.has_value);
    }

    {
        auto two = element("two");
        auto o = two.in(m);
        EXPECT_TRUE(o.has_value);
        EXPECT_EQ(2, o or n);
    }
}

TEST(The_element_function,
accesses_containers_of_optionals)
{
    std::vector<optional<int>> v{11,{},4,6,2,5};

    use_(element(3).of(v)--)_as(i){
        EXPECT_EQ(6,i);
    }
    >>[]{ ADD_FAILURE(); };

    use_(element(1).of(v)--)_as(i){
        (void)i;
        ADD_FAILURE();
    };

    std::map<std::string,optional<int>> m = {{
        {"one",1},
        {"two",2},
        {"none",{}},
    }};

    use_(element("one").in(m)--)_as(i){
        EXPECT_EQ(1,i);
    }
    >>[]{ ADD_FAILURE(); };

    element("one").in(m)
    -->>[](int i){
        EXPECT_EQ(1,i);
    }
    >>[]{
        ADD_FAILURE();
    };

    use_(element("none").in(m)--)_as(i){
        (void)i;
        ADD_FAILURE();
    };
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
