#include "optional_t.hpp"
#include <stdio.h>
#include <vector>
#include <gtest/gtest.h>

Optional<int> twice_in_range(int from, int to, int i)
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
        auto r = twice_in_range(3,7,in[i]).default_to(-1);
        EXPECT_EQ(expected[i],r);
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
            [](T v)->T {return 2*v;},
            []()->T {return -1;}
        );
        EXPECT_EQ(expected[i],r);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
