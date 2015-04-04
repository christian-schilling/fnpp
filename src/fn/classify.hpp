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

template<typename T1, typename T2>
struct ValuePair
{
    T1 first;
    T2 second;
};

struct DefaultTo {};


template<typename M, typename V>
auto match_helper(M const& m, ValuePair<DefaultTo,V> vp)
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

static auto const default_to = when(fn_::DefaultTo());

} // namespace fn


#endif
