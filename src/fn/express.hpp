#ifndef _7bdd4713_73e2_483e_b4eb_3512c4334c32
#define _7bdd4713_73e2_483e_b4eb_3512c4334c32

namespace fn {
namespace fn_ {

struct Placeholder
{
    template<typename O>
    auto operator()(O o) const -> O { return o; }
};

template<typename N>
struct PNode
{
    N n;

    template<typename T>
    auto operator()(T t) const -> decltype(n(t)) { return n(t); }
};

#define FN_MAKE_OP(NAME_,OP_)\
template<typename T>\
class NAME_\
{\
    T const v;\
\
public:\
    NAME_(T v): v(v) {}\
\
    template<typename O>\
    auto operator()(O const& o) const -> decltype(v OP_ o) { return v OP_ o; }\
};\
\
template<typename T>\
auto operator OP_(Placeholder const&, T const& i) -> PNode<NAME_<T>> const\
{\
    return PNode<NAME_<T>>{NAME_<T>(i)};\
}\
\
template<typename T>\
auto operator OP_(T const& i, Placeholder const&) -> PNode<NAME_<T>> const\
{\
    return PNode<NAME_<T>>{NAME_<T>(i)};\
}\

#define FN_MAKE_OP_PAIR(NAME1_,OP1_,NAME2_,OP2_)\
template<typename T>\
class NAME1_\
{\
    T const v;\
\
public:\
    NAME1_(T v): v(v) {}\
    auto operator()(T const& o) const -> decltype(v OP1_ o) { return v OP1_ o; }\
};\
template<typename T>\
class NAME2_\
{\
    T const v;\
\
public:\
    NAME2_(T v): v(v) {}\
    auto operator()(T const& o) const -> decltype(v OP2_ o) { return v OP2_ o; }\
};\
\
template<typename T>\
auto operator OP1_(Placeholder const&, T const& i) -> NAME2_<T> const\
{\
    return NAME2_<T>(i);\
}\
template<typename T>\
auto operator OP1_(T const& i, Placeholder const&) -> NAME1_<T> const\
{\
    return NAME1_<T>(i);\
}\
template<typename T>\
auto operator OP2_(Placeholder const&, T const& i) -> NAME1_<T> const\
{\
    return NAME1_<T>(i);\
}\
template<typename T>\
auto operator OP2_(T const& i, Placeholder const&) -> NAME2_<T> const\
{\
    return NAME2_<T>(i);\
}

FN_MAKE_OP(equal_to,==);
FN_MAKE_OP(not_equal_to,!=);
FN_MAKE_OP_PAIR(less_than,<,greater_than,>);
FN_MAKE_OP_PAIR(less_or_equal,<=,greater_or_equal,>=);

FN_MAKE_OP(addition,+);

#undef FN_MAKE_OP
#undef FN_MAKE_OP_PAIR

} // namespace fn_

static auto const _ = fn_::Placeholder();

} // namespace fn


#endif
