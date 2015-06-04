#include <cstdio>
#include <fn++.hpp>
#include <vector>
#include <map>
#include <utility>
#include "catch.hpp"
using namespace fn;


TEST_CASE("slice")
{
    SECTION("fixed_offset")
    {
        uint8_t buf[6] = {0,1,2,3,4,100};
        auto s5 = slice<uint8_t,5>::from_pointer(buf);
        auto s4 = slice<uint8_t,4>(s5.offset<1>());
        auto s4_d = s5.offset(1);
        REQUIRE(s4[3].valid());
        REQUIRE_FALSE(s4[4].valid());
        REQUIRE(4 == ~s4[3]);
        REQUIRE(0 == ~s4[4]);
        REQUIRE(4 == s4.at<3>());
        REQUIRE(4 == s4.size());

        REQUIRE(s4_d[3].valid());
        REQUIRE_FALSE(s4_d[4].valid());
        REQUIRE(4 == ~s4_d[3]);
        REQUIRE(0 == ~s4_d[4]);
        REQUIRE(4 == s4_d.size());
    }

    SECTION("dynamic_offset")
    {
        uint8_t buf[6] = {0,1,2,3,4,100};
        auto s5 = slice<uint8_t>(buf,5);
        auto s4_d = s5.offset(1);

        REQUIRE(s4_d[3].valid());
        REQUIRE_FALSE(s4_d[4].valid());
        REQUIRE(4 == ~s4_d[3]);
        REQUIRE(0 == ~s4_d[4]);
        REQUIRE(4 == s4_d.size());
    }

    SECTION("sizes_and_contents")
    {
        uint8_t buf[6] = {0,1,2,3,4,100};
        auto s5 = slice<uint8_t,5>::from_pointer(buf);
        REQUIRE(s5[3].valid());
        REQUIRE(3 == ~s5[3]);
        REQUIRE(5 == s5.size());

        auto ss = slice<uint8_t>(s5);
        REQUIRE(1 == ~ss[1]);
        REQUIRE(5 == ss.size());

        {
            size_t i=0;
            for(auto&& x: ss){
                REQUIRE(x == ~ss[i]);
                i++;
            }
            REQUIRE(i == 5);
        }

        {
            size_t i=0;
            for(auto&& x: s5){
                REQUIRE(x == ~ss[i]);
                i++;
            }
            REQUIRE(i == 5);
        }
    }


    SECTION("creation_from_std_vector")
    {
        std::vector<int> v = {4,5,6,3};

        auto sv = slice<int>(v);
        REQUIRE(5 == ~sv[1]);
        REQUIRE(4 == sv.size());
    }


    SECTION("fill")
    {
        uint8_t buf[6] = {0,1,2,3,4,100};
        auto s5 = slice<uint8_t,5>::from_pointer(buf);
        auto s4 = slice<uint8_t,4>(s5.offset<1>());
        s4.fill(11);

        REQUIRE(11 == ~s5[3]);
        REQUIRE(11 == ~s4[3]);
        REQUIRE(0 == ~s5[0]);
    }

    SECTION("fixed_copysame_size")
    {
        uint8_t buf0[6] = {0,1,2,3,4,5};
        uint8_t buf1[6] = {9,8,7,6,5,4};
        auto s0 = slice<uint8_t,5>::from_pointer(buf0);
        auto s1 = slice<uint8_t,5>::from_pointer(buf1);

        s0.copy(s1);

        REQUIRE(9 == buf0[0]);
        REQUIRE(8 == buf0[1]);
        REQUIRE(7 == buf0[2]);
        REQUIRE(6 == buf0[3]);
        REQUIRE(5 == buf0[4]);
        REQUIRE(5 == buf0[5]);
    }

    SECTION("fixed_copydifferent_size")
    {
        uint8_t buf0[6] = {0,1,2,3,4,5};
        uint8_t buf1[6] = {9,8,7,6,5,4};
        auto s0 = slice<uint8_t,5>::from_pointer(buf0);
        auto s1 = slice<uint8_t,6>(buf1);

        s0.copy(s1);

        REQUIRE(9 == buf0[0]);
        REQUIRE(8 == buf0[1]);
        REQUIRE(7 == buf0[2]);
        REQUIRE(6 == buf0[3]);
        REQUIRE(5 == buf0[4]);
        REQUIRE(5 == buf0[5]);
    }


    SECTION("dynamic_copysame_size")
    {
        uint8_t buf0[6] = {0,1,2,3,4,5};
        uint8_t buf1[6] = {9,8,7,6,5,4};
        auto s0 = slice<uint8_t>(buf0,5);
        auto s1 = slice<uint8_t>(buf1,5);

        s0.copy(s1);

        REQUIRE(9 == buf0[0]);
        REQUIRE(8 == buf0[1]);
        REQUIRE(7 == buf0[2]);
        REQUIRE(6 == buf0[3]);
        REQUIRE(5 == buf0[4]);
        REQUIRE(5 == buf0[5]);
    }

    SECTION("dynamic_copydifferent_size")
    {
        uint8_t buf0[6] = {0,1,2,3,4,5};
        uint8_t buf1[6] = {9,8,7,6,5,4};
        auto s0 = slice<uint8_t>(buf0,5);
        auto s1 = slice<uint8_t>(buf1,3);

        s0.copy(s1);

        REQUIRE(9 == buf0[0]);
        REQUIRE(8 == buf0[1]);
        REQUIRE(7 == buf0[2]);
        REQUIRE(3 == buf0[3]);
        REQUIRE(4 == buf0[4]);
        REQUIRE(5 == buf0[5]);
    }

    SECTION("dynamic_to_fixed")
    {
        uint8_t buf[6] = {0,1,2,3,4,100};
        auto ds = slice<uint8_t>(buf,5);

        {
            auto fs = ds.subslice<4>();
            REQUIRE(fs.valid());
        }

        {
            auto fs = ds.subslice<10>();
            REQUIRE_FALSE(fs.valid());
        }
    }

    SECTION("dynamic_subslice")
    {
        uint8_t buf[6] = {0,1,2,3,4,100};
        auto ds = slice<uint8_t>(buf,5);

        {
            auto fs = ds.subslice(4);
            REQUIRE(4 == fs.size());
        }

        {
            auto fs = ds.subslice(10);
            REQUIRE(0 == fs.size());
        }
    }

    SECTION("fixed_to_dynamic")
    {
        uint8_t buf[6] = {0,1,2,3,4,100};
        auto fs = slice<uint8_t,5>::from_pointer(buf);

        {
            auto ds = slice<uint8_t>(fs);
            REQUIRE(5 == ds.size());
        }
    }

    SECTION("reassign_fixed")
    {
        uint8_t buf0[6] = {0,1,2,3,4,5};
        uint8_t buf1[6] = {9,8,7,6,5,4};

        auto s = slice<uint8_t,4>(buf0);

        REQUIRE(0 == s.at<0>());
        REQUIRE(1 == s.at<1>());

        s = slice<uint8_t,4>(buf1);

        REQUIRE(9 == s.at<0>());
        REQUIRE(8 == s.at<1>());
    }

    SECTION("reassign_dynamic")
    {
        uint8_t buf0[6] = {0,1,2,3,4,5};
        uint8_t buf1[6] = {9,8,7,6,5,4};

        auto s = slice<uint8_t>(buf0,4);

        REQUIRE(0 == ~s[0]);
        REQUIRE(1 == ~s[1]);

        s = slice<uint8_t>(buf1,4);

        REQUIRE(9 == ~s[0]);
        REQUIRE(8 == ~s[1]);
    }

    struct Structure
    {
        uint16_t s0;
        uint8_t s1;
    };

    SECTION("fixed_reinterpret")
    {
        uint8_t buf[] = {0,1,2,3,4,5,6,7,8,9,10,0,0,0,0,0,0,0,0,0,0,0};
        auto fs = slice<uint8_t,13>::from_pointer(buf);

        printf("sizeof Structure: %d\n",int(sizeof(Structure)));

        {
            auto rs = fs.reinterpret_as<Structure>();
            REQUIRE(3 == rs.size());
        }

        {
            auto rs = fs.reinterpret_as<Structure,1>();
            REQUIRE(1 == rs.size());
        }

        {
            auto rs = fs.reinterpret_as<Structure,2>();
            REQUIRE(2 == rs.size());
        }

        // should not compile
        /* { */
        /*     auto rs = fs.reinterpret_as<Structure,4>(); */
        /*     REQUIRE(4 == rs.size()); */
        /* } */
    }

    SECTION("dynamic_reinterpret")
    {
        uint8_t buf[] = {0,1,2,3,4,5,6,7,8,9,10,0,0,0,0,0,0,0,0,0,0,0};
        auto fs = slice<uint8_t>(buf,13);

        printf("sizeof Structure: %d\n",int(sizeof(Structure)));

        {
            auto rs = fs.reinterpret_as<Structure>();
            REQUIRE(3 == rs.size());
        }

        {
            auto rs = fs.reinterpret_as<Structure>(1);
            REQUIRE(1 == rs.size());
        }

        {
            auto rs = fs.reinterpret_as<Structure>(2);
            REQUIRE(2 == rs.size());
        }

        {
            auto rs = fs.reinterpret_as<Structure>(4);
            REQUIRE(0 == rs.size());
        }
    }

    SECTION("make_slice")
    {
        std::vector<int> v = {4,5,6,3};

        slice<int> s = make_slice(v);
        REQUIRE(s.size() == v.size());
    }

    SECTION("make_slice_fixed")
    {
        std::vector<int> v = {4,5,6,3};

        slice<int,3> s = slice_from_pointer<3>(v.data());
        REQUIRE(3 == s.size());
    }
}
