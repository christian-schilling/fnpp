#include "funny.hpp"
#include <stdio.h>
#include <vector>
#include <utility>
#include <gtest/gtest.h>

using namespace funny;

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
keeps_beeing_optional_if_only_the_exists_case_is_handled)
{
    std::vector<int> in{1,4,6,2,10,55};
    std::vector<int> expected{-1,8,12,-1,-1,-1};
    std::vector<int> expected2{0,8*3,12*3,0,0,0};
    for(size_t i=0;i<in.size();i++){
        int out = 0;
        optional<int> const& result = twice_in_range(3,7,in[i]);
        optional<int> const& still_optional = result.use([&](int v){out=v*3;});
        auto r = still_optional or -1;
        EXPECT_EQ(expected[i],r);
        EXPECT_EQ(expected2[i],out);
    }
}


TEST(An_optional_value,
can_be_used_by_providing_handlers_for_both_cases)
{
    std::vector<int> in{1,4,6,2,10,55};
    std::vector<int> expected{-1,16,24,-1,-1,-1};
    for(size_t i=0;i<in.size();i++){
        auto result = twice_in_range(3,7,in[i]);
        typedef decltype(result)::Type T;
        auto r = result.use(
            [](T v){return 2*v;},
            [](){return -1;}
        );
        EXPECT_EQ(expected[i],r);
    }
}

TEST(An_optional_value,
can_be_converted_to_bool_to_test_if_it_has_a_value)
{
    auto t = optional<int>{1};
    EXPECT_TRUE(t);

    auto t0 = optional<int>{0};
    EXPECT_TRUE(t0);

    auto f = optional<int>{};
    EXPECT_FALSE(f);
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
