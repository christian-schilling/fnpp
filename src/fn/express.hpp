#ifndef _7bdd4713_73e2_483e_b4eb_3512c4334c32
#define _7bdd4713_73e2_483e_b4eb_3512c4334c32

namespace fn {
namespace fn_ {

struct Placeholder
{
    template<typename R, typename L, typename V>
    auto operator()(R, L, V v) const -> V { return v; }
};

template<typename T>
struct Constant
{
    T v;
    template<typename R, typename L, typename V>
    auto operator()(R, L, V) const -> T { return v; }
};

template<typename L, typename R, typename Op>
struct ENode
{
    L l;
    R r;
    Op op;

    template<typename V>
    auto operator()(V v) const -> decltype(op(l,r,v)) { return op(l,r,v); }
};


#define FN_MAKE_OP(NAME_,OP_)\
struct NAME_\
{\
    template<typename R, typename L, typename V>\
    auto operator()(L l, R r, V v) const -> decltype(l(v)  OP_ r(v))\
    {\
        return l(v)  OP_ r(v);\
    }\
\
};\
template<\
    typename LL, typename LR, typename LOp,\
    typename RL, typename RR, typename ROp\
>\
auto operator OP_(ENode<LL,LR,LOp> lnode, ENode<RL,RR,ROp> rnode)\
    -> ENode<ENode<LL,LR,LOp>,ENode<RL,RR,ROp>,NAME_>\
{\
    return ENode<ENode<LL,LR,LOp>,ENode<RL,RR,ROp>,NAME_>{lnode,rnode,NAME_()};\
}\
template<\
    typename LL, typename LR, typename LOp,\
    typename T\
>\
auto operator OP_(ENode<LL,LR,LOp> lnode, T v)\
    -> ENode<ENode<LL,LR,LOp>,ENode<Placeholder,Placeholder,Constant<T>>,NAME_>\
{\
    return ENode<\
        ENode<LL,LR,LOp>,\
        ENode<Placeholder,Placeholder,Constant<T>>,\
        NAME_\
    >{lnode,{{},{},{v}},NAME_()};\
}\
template<\
    typename RL, typename RR, typename ROp,\
    typename T\
>\
auto operator OP_(T v,ENode<RL,RR,ROp> rnode)\
    -> ENode<ENode<Placeholder,Placeholder,Constant<T>>,ENode<RL,RR,ROp>,NAME_>\
{\
    return ENode<\
        ENode<Placeholder,Placeholder,Constant<T>>,\
        ENode<RL,RR,ROp>,\
        NAME_\
    >{{{},{},{v}},rnode,NAME_()};\
}

FN_MAKE_OP(fn_op_addition,+)
FN_MAKE_OP(fn_op_multiplication,*)
FN_MAKE_OP(fn_op_division,/)
FN_MAKE_OP(fn_op_modulo,%)
FN_MAKE_OP(fn_op_subtraction,-)
FN_MAKE_OP(fn_op_greater,>)
FN_MAKE_OP(fn_op_smaller,<)
FN_MAKE_OP(fn_op_greater_or_equal,>=)
FN_MAKE_OP(fn_op_smaller_or_equal,<=)
FN_MAKE_OP(fn_op_equal,==)
FN_MAKE_OP(fn_op_not_equal,!=)
FN_MAKE_OP(fn_op_rshift,>>)
FN_MAKE_OP(fn_op_lshift,<<)
FN_MAKE_OP(fn_op_and,&&)
FN_MAKE_OP(fn_op_or,||)
FN_MAKE_OP(fn_op_bit_and,&)
FN_MAKE_OP(fn_op_bit_or,|)
FN_MAKE_OP(fn_op_bit_xor,^)
#undef FN_MAKE_OP

#define FN_MAKE_UNARY_OP(NAME_,OP_)\
struct NAME_\
{\
    template<typename R, typename L, typename V>\
    auto operator()(L, R r, V v) const -> decltype(OP_ r(v)) { return OP_ r(v); }\
};\
\
template<\
    typename RL, typename RR, typename ROp\
>\
auto operator OP_(ENode<RL,RR,ROp> rnode)\
    -> ENode<ENode<Placeholder,Placeholder,Placeholder>,ENode<RL,RR,ROp>,NAME_>\
{\
    return ENode<\
        ENode<Placeholder,Placeholder,Placeholder>,\
        ENode<RL,RR,ROp>,\
        NAME_\
    >{{},rnode,NAME_()};\
}

FN_MAKE_UNARY_OP(negate,!)
FN_MAKE_UNARY_OP(bitwise_negate,~)

#undef FN_MAKE_UNARY_OP

} // namespace fn_

static auto const _ = fn_::ENode<
    fn_::Placeholder,
    fn_::Placeholder,
    fn_::Placeholder
>();

} // namespace fn


#endif
