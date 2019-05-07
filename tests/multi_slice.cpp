
#include <fn/multi_slice.hpp>

#include "catch.hpp"

using namespace fn;
typedef multi_slice<uint8_t, const uint16_t> MS816;

std::array<uint8_t, 10> u8_array         = {{0}};
std::array<const uint16_t, 10> u16_array = {{0}}; // const to make sure multi_slice is usable with const as well

// Check that conditional return type is correct in all cases, meaning is non-const if and
// only if both the requested type as well as the listed type are non-const
static_assert((std::is_same<return_T<TL_id<MS816::types, uint8_t> >::type, uint8_t>::value), "");
static_assert((std::is_same<return_T<TL_id<MS816::types, const uint8_t> >::type, const uint8_t>::value), "");
static_assert((std::is_same<return_T<TL_id<MS816::types, const uint16_t> >::type, const uint16_t>::value), "");
static_assert((std::is_same<return_T<TL_id<MS816::types, uint16_t> >::type, const uint16_t>::value), "");

TEST_CASE("multi_slice, constructed_from_slices_of_different_sizes_has_size_zero")
{
    std::array<uint16_t, 14> u16_array_14;

    MS816 u8u16 = MS816(slice<uint8_t>(u8_array), slice<uint16_t>(u16_array_14));
    REQUIRE(0U == u8u16.size());
}

TEST_CASE("multi_slice, constructed_from_slices_of_matching_sizes_has_same_size")
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    REQUIRE(10U == u8u16.size());
    REQUIRE(10U == u8u16.get<uint8_t>().size());
    REQUIRE(10U == u8u16.get<const uint16_t>().size());
}

TEST_CASE("multi_slice, constructed_from_slices_of_matching_sizes_has_same_different_size")
{
    std::array<uint8_t, 5> u8_array_5;
    std::array<uint16_t, 5> u16_array_5;
    MS816 u8u16 = MS816(u8_array_5, u16_array_5);
    REQUIRE(5U == u8u16.size());
    REQUIRE(5U == u8u16.get<uint8_t>().size());
    REQUIRE(5U == u8u16.get<const uint16_t>().size());
}

TEST_CASE("multi_slice, constructed_from_slices_of_matching_sizes_has_data_pointers_initialized")
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    REQUIRE(u8_array.data() == u8u16.data<uint8_t>());
    REQUIRE(u8_array.data() == u8u16.data<const uint8_t>());
    REQUIRE(u16_array.data() == u8u16.data<uint16_t>());
    REQUIRE(u16_array.data() == u8u16.data<const uint16_t>());
}

TEST_CASE("multi_slice, trim_reduces_the_size")
{
    MS816 u8u16 = MS816(u8_array, u16_array);

    u8u16.trim(6U);

    REQUIRE(6U == u8u16.size());
    REQUIRE(6U == u8u16.get<uint8_t>().size());
    REQUIRE(6U == u8u16.get<const uint16_t>().size());
}

TEST_CASE("multi_slice, trim_sets_size_to_zero_if_passed_value_is_bigger_than_size")
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    u8u16.trim(12);
    REQUIRE(0U == u8u16.size());
}

TEST_CASE("multi_slice, advance_reduces_size")
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    u8u16.advance(6);
    REQUIRE(4U == u8u16.size());
}

TEST_CASE("multi_slice, advance_adjusts_data_pointers")
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    u8u16.advance(6);
    REQUIRE((u8_array.data() + 6U) == u8u16.get<uint8_t>().data());
    REQUIRE((u16_array.data() + 6U) == u8u16.get<const uint16_t>().data());
}

TEST_CASE("multi_slice, advance_sets_size_to_zero_if_size_is_to_big")
{
    MS816 u8u16 = MS816(u8_array, u16_array);
    u8u16.advance(11);
    REQUIRE(0U == u8u16.size());
}

TEST_CASE("multi_slice, can_be_converted_to_a_subset_multi_slice")
{
    std::array<uint8_t, 10> a8;
    std::array<uint16_t, 10> a16;
    std::array<uint32_t, 10> a32;

    // Note that it also converts "uint16_t" -> "const uint16_t"
    multi_slice<uint16_t, uint8_t, uint32_t> c(a16, a8, a32);
    MS816 u8u16 = MS816(c);

    REQUIRE(10U == u8u16.size());
    REQUIRE(a16.data() == u8u16.data<const uint16_t>());
    REQUIRE(a8.data() == u8u16.data<uint8_t>());
}

TEST_CASE("multi_slice, idx_gives_the_position_of_an_item")
{
    MS816 u8u16 = MS816(u8_array, u16_array);

    REQUIRE(3 == u8u16.idx(&u8_array[3]));
    REQUIRE(4 == u8u16.idx(&u16_array[4]));
}

TEST_CASE("multi_slice, at_returns_reference_to_an_item")
{
    MS816 u8u16 = MS816(u8_array, u16_array);

    REQUIRE(&u8_array[2] == &u8u16.at<uint8_t>(2));
    REQUIRE(&u16_array[5] == &u8u16.at<uint16_t>(5));
}

template <size_t>
struct V
{
    V() : value(0) {}
    V(int v) : value(v) {}
    int value;
};

TEST_CASE("multi_slice, all_the_overloads_want_to_be_covered")
{
    std::array<V<0>, 5> v0;
    std::array<V<1>, 5> v1;
    std::array<V<2>, 5> v2;
    std::array<V<3>, 5> v3;
    std::array<V<4>, 5> v4;
    std::array<V<5>, 5> v5;
    std::array<V<6>, 5> v6;
    std::array<V<7>, 5> v7;

    multi_slice<V<0>, V<1> > s2(v0, v1);
    multi_slice<V<0>, V<1>, V<2> > s3(v0, v1, v2);
    multi_slice<V<0>, V<1>, V<2>, V<3> > s4(v0, v1, v2, v3);
    multi_slice<V<0>, V<1>, V<2>, V<3>, V<4> > s5(v0, v1, v2, v3, v4);
    multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5> > s6(v0, v1, v2, v3, v4, v5);
    multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>, V<6> > s7(v0, v1, v2, v3, v4, v5, v6);
    multi_slice<V<0>, V<1>, V<2>, V<3>, V<4>, V<5>, V<6>, V<7> > s8(v0, v1, v2, v3, v4, v5, v6, v7);

    REQUIRE(0U == s2.advance(11).size());
    REQUIRE(0U == s3.advance(11).size());
    REQUIRE(0U == s4.advance(11).size());
    REQUIRE(0U == s5.advance(11).size());
    REQUIRE(0U == s6.advance(11).size());
    REQUIRE(0U == s7.advance(11).size());
}

