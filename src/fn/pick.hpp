#ifndef _71e78c5d_ea88_4df2_977c_1a40712c3291
#define _71e78c5d_ea88_4df2_977c_1a40712c3291

namespace fn {

namespace fn_ {

template<typename F>
struct Invoke
{
    F f;
    template<typename ...Args>
    auto operator()(Args... args) -> decltype(f(args...)) { return f(args...); }
};

template<typename V>
struct InvokeHelper
{
    template<typename X>
    static auto inv(V v,X) -> V { return v; }
};

template<typename V>
struct InvokeHelper<Invoke<V>>
{
    template<typename X>
    static auto inv(Invoke<V> v,X x) -> decltype(v(x)) { return v(x); }
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

template<typename T>
struct Pick
{
    T const k;

	template<typename K, typename V, typename ...Args>
	auto operator()(ValuePair<K, V> vp, Args... args) const
        -> decltype(InvokeHelper<V>::inv(vp.second,k))
    {
        return match_helper(*this,vp,args...);
    }
};

template<typename P>
struct Is { P const p; };

template<typename P, typename Then>
auto operator >>=(Is<P> const w, Then const then) -> ValuePair<P,Then> const
{
    return ValuePair<P,Then>{w.p,then};
}

template<typename P, typename Then>
auto operator >>(Is<P> const w, Then const then) -> ValuePair<P,Invoke<Then>> const
{
    return ValuePair<P,Invoke<Then>>{w.p,Invoke<Then>{then}};
}

} // namespace fn_

template<typename K>
auto pick(K t) -> fn_::Pick<K> const { return fn_::Pick<K>{t}; }

template<typename P>
auto is(P p) -> fn_::Is<P> const { return fn_::Is<P>{p}; }

static auto const default_to = is(fn_::DefaultTo());

} // namespace fn

#endif
