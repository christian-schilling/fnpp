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

template<typename Predicate, typename Value>
struct Rule
{
    Predicate predicate;
    Value value;
};

struct DefaultTo {};

template<typename P, typename Value>
auto match_helper(P const& p, Rule<DefaultTo,Value> rule)
    -> decltype(InvokeHelper<Value>::inv(rule.value,p.var))
{
    return InvokeHelper<Value>::inv(rule.value,p.var);
}

template<typename P, typename Predicate, typename Value, typename ...Args>
auto match_helper(P const& p, Rule<Predicate,Value> rule, Args... args)
    -> decltype(InvokeHelper<Value>::inv(rule.value,p.var))
{
    if(rule.predicate(p.var)){
        return InvokeHelper<Value>::inv(rule.value,p.var);
    }
    else{
        return match_helper(p,args...);
    }
}

template<typename Var>
struct Match
{
    Var const var;

	template<typename Predicate, typename Value, typename ...Args>
	auto operator()(Rule<Predicate, Value> rule, Args... args) const
        -> decltype(InvokeHelper<Value>::inv(rule.value,var))
    {
        return match_helper(*this,rule,args...);
    }
};

template<typename Predicate>
struct When { Predicate const predicate; };

template<typename Predicate, typename Then>
auto operator >>=(When<Predicate> const when, Then const then)
    -> Rule<Predicate,Then> const
{
    return Rule<Predicate,Then>{when.predicate,then};
}

template<typename Predicate, typename Then>
auto operator >>(When<Predicate> const when, Then const then)
    -> Rule<Predicate,Invoke<Then>> const
{
    return Rule<Predicate,Invoke<Then>>{when.predicate,Invoke<Then>{then}};
}

// Specialization for expression nodes
template<typename L, typename R, typename Op> struct ENode;
template<typename Predicate, typename L, typename R, typename Op>
auto operator >>=(When<Predicate> const when, ENode<L,R,Op> const then)
    -> Rule<Predicate,Invoke<ENode<L,R,Op>>> const
{
    return Rule<Predicate,Invoke<ENode<L,R,Op>>>{
        when.predicate,Invoke<ENode<L,R,Op>>{then}
    };
}



} // namespace fn_

template<typename Var>
auto match(Var var) -> fn_::Match<Var> const { return fn_::Match<Var>{var}; }

template<typename Predicate>
auto when(Predicate p) -> fn_::When<Predicate> const { return fn_::When<Predicate>{p}; }

static auto const default_to = when(fn_::DefaultTo());

} // namespace fn

#endif
