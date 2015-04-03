#ifndef _71e78c5d_ea88_4df2_977c_1a40712c3291
#define _71e78c5d_ea88_4df2_977c_1a40712c3291

namespace fn {

namespace fn_ {

template<typename F>
class Invoke
{
    F f;

public:
    Invoke(F f): f(f) {}

    template<typename ...Args>
    auto operator()(Args... args) -> decltype(f(args...))
    {
        return f(args...);
    }
};

template<typename V>
struct InvokeHelper
{
    template<typename X>
    static auto inv(V v,X) -> V
    {
        return v;
    }
};

template<typename V>
struct InvokeHelper<Invoke<V>>
{
    template<typename X>
    static auto inv(Invoke<V> v,X x) -> decltype(v(x))
    {
        return v(x);
    }
};

struct Placeholder
{
    template<typename O>
    auto operator()(O o) const -> O { return o; }
};

template<typename T1, typename T2>
struct ValuePair
{
    T1 first;
    T2 second;
};


template<typename M, typename V>
auto match_helper(M const& m, ValuePair<Placeholder,V> vp)
    -> decltype(InvokeHelper<V>::inv(vp.second,m.k))
{
    return InvokeHelper<V>::inv(vp.second,m.k);
}

template<typename M, typename K, typename V, typename ...Args>
auto match_helper(M const& m, ValuePair<K,V> vp, Args... args)
    -> decltype(InvokeHelper<V>::inv(vp.second,m.k))
{
    if(vp.first(m.k)){
        return InvokeHelper<V>::inv(vp.second,m.k);
    }
    else{
        return match_helper(m,args...);
    }
}

template<typename K>
struct Match
{
    K const k;

    template<typename ...Args>
    auto operator()(Args... args) const -> decltype(match_helper(*this,args...))
    {
        return match_helper(*this,args...);
    }
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

template<typename P>
struct Is { P const p; };

template<typename P, typename Then>
auto operator >>=(Is<P> const isp, Then const then) -> ValuePair<P,Then> const
{
    return ValuePair<P,Then>{isp.p,then};
}

template<typename P, typename Then>
auto operator >>(Is<P> const isp, Then const then) -> ValuePair<P,Invoke<Then>> const
{
    return ValuePair<P,Invoke<Then>>{isp.p,Invoke<Then>(then)};
}

} // namespace fn_

template<typename K>
auto classify(K t) -> fn_::Match<K> const { return fn_::Match<K>{t}; }

template<typename P>
auto when(P p) -> fn_::Is<P> const { return fn_::Is<P>{p}; }

static auto const _ = fn_::Placeholder();
static auto const default_to = when(_);

} // namespace fn


#endif
