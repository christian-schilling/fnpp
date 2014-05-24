#ifndef _1af7690b_7fca_4464_9e14_c71bc5a29ee9
#define _1af7690b_7fca_4464_9e14_c71bc5a29ee9

#include <stdio.h>

namespace fn{

namespace fn_ {

template< class R > struct remove_reference      {typedef R T;};
template< class R > struct remove_reference<R&>  {typedef R T;};
template< class R > struct remove_reference<R&&> {typedef R T;};

template<typename C>
struct noconst{ typedef C T; };
template<typename B>
struct noconst<B const&>{ typedef B T; };

template<typename T>
struct IsTrue{ bool operator()(T i)const {return static_cast<bool>(i);} };

#ifndef _MSC_VER
template<typename T>
constexpr bool is_void() {return false;}
template<>
constexpr bool is_void<void>() {return true;}
#else
#define or ||
#endif

template<typename T>
class Range
{
    class IT
    {
    public:
        typedef T value_type;
        IT(T const& position): position(position) {}
        IT(T const& position, T const& step): position(position),step(step) {}
        bool operator!=(IT const& other)const {return position!=other.position;}
        IT const& operator++() {position+=step;return *this;}
        T const& operator*()const {return position;}
    private:
        T position;
        T step = 1;
    };

    IT from;
    IT const to;
public:
    Range(T const& to): from(0), to(to) {}
    IT const& begin()const { return from; }
    IT const& end()const { return to; }
};};

template<typename T,typename ...Args>
auto range(T const& t,Args const& ...args) -> fn_::Range<T>{
    return fn_::Range<T>(t,args...);
}

namespace fn_ {
template<typename FN,typename G, typename OtherIT>
class Map
{
    class IT
    {
    private:
        FN const& fn;
        OtherIT other_it;
    public:
        IT(FN fn,OtherIT other_it): fn(fn), other_it(other_it) {}
        bool operator!=(IT& other){return other_it!=other.other_it;}
        IT const& operator++() {++other_it;return *this;}
        auto operator*()const -> decltype(fn(*other_it)) { return fn(*other_it); }
    };

    IT const from;
    IT const to;

public:
    Map(FN fn, G const& g): from(fn,g.begin()), to(fn,g.end()) {}
    IT const& begin() const { return from; }
    IT const& end() const { return to; }
};};

template<typename FN,typename G>
auto map(FN const& fn,G const& g) -> fn_::Map<FN,G,
    typename fn_::noconst<decltype(g.begin())>::T>{
    return fn_::Map<FN,G,typename fn_::noconst<decltype(g.begin())>::T>(fn,g);
}

namespace fn_ {
template<typename OtherIT>
class AsRange
{
    OtherIT const from;
    OtherIT const to;

public:
    AsRange(OtherIT from, OtherIT to): from(from), to(to) {}
    OtherIT const& begin() const { return from; }
    OtherIT const& end() const { return to; }
};};

template<typename OtherIT>
auto as_range(OtherIT const& b, OtherIT const& e) -> fn_::AsRange<OtherIT>{
    return fn_::AsRange<OtherIT>(b,e);
}

namespace fn_ {
template<
    template<typename,typename> class PairT,
    typename A,
    typename B,
    typename OtherIT1,
    typename OtherIT2
>
class Zip
{
    class IT
    {
    private:
        OtherIT1 other_it1;
        OtherIT2 other_it2;
        typedef typename OtherIT1::value_type const OtherVal1;
        typedef typename OtherIT2::value_type const OtherVal2;
        typedef PairT<OtherVal1,OtherVal2> Pair;

    public:
        typedef Pair value_type;

        IT(OtherIT1 other_it1,OtherIT2 other_it2):
            other_it1(other_it1),other_it2(other_it2) {}
        bool operator!=(IT& other){
            return (
                (other_it1!=other.other_it1) && (other_it2 != other.other_it2)
            );
        }
        IT const& operator++() {++other_it1;++other_it2;return *this;}

        Pair operator*() {
            return Pair(*other_it1,*other_it2);
        }
    };

    IT const from;
    IT const to;

public:
    Zip(A const& a, B const& b):
        from(a.begin(),b.begin()), to(a.end(),b.end()) {}
    IT const& begin() const { return from; }
    IT const& end() const { return to; }
};};

namespace fn_ {
template<typename A, typename B>
struct Pair
{
    Pair(A& a, B& b):
        first(a),second(b),
        key(a),value(b),
        nr(a),item(b)
    {}

    A& first;
    B& second;

    A& key;
    B& value;

    A& nr;
    B& item;
};};

template<template<typename,typename> class PairT=fn_::Pair, typename A,typename B>
auto zip(A const& a,B const& b) -> fn_::Zip<PairT,A,B,
        typename fn_::noconst<decltype(a.begin())>::T,
        typename fn_::noconst<decltype(b.begin())>::T
    >{
    return fn_::Zip<PairT,A,B,
        typename fn_::noconst<decltype(a.begin())>::T,
        typename fn_::noconst<decltype(b.begin())>::T
    >(a,b);
}

template<typename A>
auto enumerate(A const& a)
     ->decltype(zip(range(-1),a))
{
    return zip(range(-1),a);
}

namespace fn_ {
template<typename FN,typename G, typename OtherIT>
class Filter
{
    class IT
    {
    private:
        FN const& fn;
        OtherIT other_it;
    public:
        IT(FN fn,OtherIT other_it): fn(fn), other_it(other_it){}
        bool operator!=(IT& other){
            while(other_it!=other.other_it && !fn(*other_it)){ ++other_it; }
            return other_it!=other.other_it;
        }
        IT const& operator++() { ++other_it; return *this; }
        auto operator*()const -> decltype(*other_it) { return *other_it; }
    };

    IT const from;
    IT const to;

public:
    Filter(FN fn, G const& g): from(fn,g.begin()), to(fn,g.end()) {}
    IT const& begin() const { return from; }
    IT const& end() const { return to; }
};};

template<typename FN,typename G>
auto filter(FN const& fn,G const& g) -> fn_::Filter<FN,G,
    typename fn_::noconst<decltype(g.begin())>::T>
{
    return fn_::Filter<FN,G,typename fn_::noconst<decltype(g.begin())>::T>(fn,g);
}

template<typename G>
auto filter(G const& g)
     -> decltype(filter(fn_::IsTrue<decltype(*g.begin())>{},(*g.begin(),g)))
{
    return filter(fn_::IsTrue<decltype(*g.begin())>{},g);
}

template<typename I,typename T, typename F>
T reduce(I const& iter, T const& neutral, F const& f)
{
    T v = neutral;
    for(auto const& x: iter){
        v = f(v,x);
    }
    return v;
}


namespace fn_ {

template< class T > struct optional_ref_helper     {static T   h(T&)  {return {};}};
template< class T > struct optional_ref_helper<T&> {static T&  h(T& t)  {return t;}};
template< class T > struct optional_ref_helper<T&&>{static T&& h(T&& t) {return t;}};

template<typename O, typename ValueF, typename T>
class optional_helper
{
    O o;
    T handler_returned;

public:
    optional_helper(O const& o,typename O::Type const& o_value, ValueF const& handle_value):
        o(o),
        handler_returned(o.has_value
                ? handle_value(const_cast<typename O::Type&>(o_value))
                : fn_::optional_ref_helper<T>::h(handler_returned))
    {}

    template<typename EmptyF>
    auto operator>>(EmptyF const& handle_no_value) const
        ->decltype(handle_no_value())
    {
        return o.has_value
            ? handler_returned
            : handle_no_value();
    }
};

template<typename O, typename ValueF>
class optional_helper<O,ValueF,void>
{
    O o;

public:
    optional_helper(O const& o,typename O::Type const& o_value, ValueF const& handle_value):
        o(o)
    {
        if(o.has_value){
            handle_value(const_cast<typename O::Type&>(o_value));
        }
    }

    template<typename EmptyF>
    auto operator>>(EmptyF const& handle_no_value) const
        ->decltype(handle_no_value())
    {
        if(!o.has_value){
            handle_no_value();
        }
    }
};


template<typename O>
class optional_helper<O,void,void>
{
    O o;

public:
    optional_helper(O const& o):
        o(o)
    {}

    template<typename EmptyF>
    auto operator>>(EmptyF const& handle_no_value) const
        ->decltype(handle_no_value())
    {
        if(!o.has_value){
            handle_no_value();
        }
    }
};


template<typename T>
class optional_base
{
public:
    bool const has_value;

protected:
    T& value;

    template<class UR>
    inline optional_base(bool has_value, UR&& value): has_value(has_value), value(const_cast<T&>(value))
    {}

public:
    typedef T Type;

    inline optional_base(optional_base const& original):
        has_value(original.has_value),
        value(original.value)
    {}

    inline optional_base(optional_base const&& original):
        has_value(original.has_value),
        value(original.value)
    {}

    template<typename F>
    inline typename fn_::remove_reference<T>::T operator||(F const& fallback) const
    {
        return has_value ? value : fallback;
    }

    inline bool operator==(T const& other_value) const
    {
        return has_value && (value == other_value);
    }

    inline bool operator!=(T const& other_value) const
    {
        return !((*this) == other_value);
    }

    inline bool operator==(optional_base const& other) const
    {
        return (has_value && other.has_value) && (value == other.value);
    }

    inline bool operator!=(optional_base const& other) const
    {
        return !(*this == other);
    }

    template<typename ValueF>
    inline auto operator>>(
        ValueF const& handle_value) const
    ->fn_::optional_helper<
        optional_base,
        ValueF,
        decltype(handle_value(const_cast<T&>(value)))
    >
    {
        return {*this, value, handle_value};
    }

    inline auto operator!()
        ->fn_::optional_helper<optional_base,void,void>
    {
        return {*this};
    }

#ifdef _MSC_VER
    optional_base& operator=(optional_base&&){return *this;}
#endif
};

template<typename T>
class optional_value : public optional_base<T>
{
    using optional_base<T>::optional_base;

protected:
    union storage
    {
        storage() {}
        storage(storage const& o): dummy(o.dummy) { }
        storage(T const& value): value(value) { }
        ~storage() {}
        char dummy;
        T value;
    };
    storage value_storage;

public:
    optional_value(T const& value):
        optional_base<T>(true,value_storage.value),
        value_storage(value)
    {}

    typename fn_::remove_reference<T>::T operator--(int)
    {
        return (*this) or typename T::Type{};
    }
};

template<typename T>
class optional_ref : public optional_base<T>
{
    using optional_base<T>::optional_base;

protected:
    using optional_base<T>::value;

public:
    using optional_base<T>::has_value;

    inline optional_ref(T& value):
        optional_base<T>(true,value)
    {}

    inline optional_ref(T&& value):
        optional_base<T>(true,value)
    {}

    typename fn_::remove_reference<T>::T operator--(int)
    {
        return has_value ? value : typename T::Type{};
    }

    template<typename F>
    inline F& operator||(F& fallback)
    {
        return has_value ? value : fallback;
    }

    template<typename F>
    inline F operator||(F const& fallback) const
    {
        return optional_base<T>::has_value ? optional_base<T>::value : fallback;
    }
};

}

template<typename T>
class optional final : public fn_::optional_value<T>
{
    using fn_::optional_value<T>::optional_value;
    using fn_::optional_value<T>::value_storage;

    friend class optional<T const>;
    friend class optional<T const&>;
    friend class optional<T&>;

public:
    optional():
        fn_::optional_value<T>(false,value_storage.value)
    {}

    optional(optional<T const&> const& original):
        optional(original.has_value,original.value)
    {}

    optional(optional<T const> const& original):
        optional(original.has_value,original.value)
    {}
};

template<typename T>
class optional<T const> final : public fn_::optional_value<T const>
{
    using fn_::optional_value<T const>::optional_value;
    using fn_::optional_value<T const>::value_storage;

    friend class optional<T>;

public:
    optional():
        fn_::optional_value<T const>(false,value_storage.value)
    {}

    optional(optional<T> const& original):
        optional(original.has_value,original.value)
    {}
};

template<typename T>
class optional<T&> final : public fn_::optional_ref<T>
{
    using fn_::optional_ref<T>::optional_ref;
    using fn_::optional_ref<T>::value;

    friend class optional<T const&>;

public:
    optional():
        fn_::optional_ref<T>(false,value)
    {}

    optional(optional<T> const& original):
        optional(original.has_value,original.value)
    {}
};

template<typename T>
class optional<T const&> final : public fn_::optional_ref<T const>
{
    using fn_::optional_ref<T const>::optional_ref;
    using fn_::optional_ref<T const>::value;

    friend class optional<T const>;
    friend class optional<T>;

public:
    optional():
        fn_::optional_ref<T const>(false,value)
    {}

    optional(optional<T> const& original):
        optional(original.has_value,original.value)
    {}

    optional(optional<T&> const& original):
        optional(original.has_value,original.value)
    {}
};


namespace fn_ {

template<class T>
class Element
{
    T const i;
public:
    Element(T const i): i{i} {}

    template<class Container>
    auto of(Container& c) ->optional<decltype(c.at(0))>
    {
        auto const size = c.size();
        auto index = static_cast<decltype(size)>(i<0?(size+i):i);
        if(index < size){
            return c.at(index);
        }
        else{
            return {};
        }
        (void)c.back(); // protect against using with std::map
    }

    template<class Container>
    auto in(Container& c) ->optional<decltype(c.at(i))>
    {
        if(c.count(i)){
            return c.at(i);
        }
        else{
            return {};
        }
    }
};
}

template<class T>
fn_::Element<T> element(T const i)
{
    return fn_::Element<T>(i);
}

}

#ifndef _MSC_VER
#define FN_TYPENAME typename
#else
#define FN_TYPENAME
#endif

#define FN_OTYPE(X) FN_TYPENAME fn::fn_::remove_reference<decltype(X)>::T::Type
#define use_(X) X >>[&](FN_OTYPE(X)&
#define _as(X) X)

#define with_(X) use_(X)_as(X)

#endif
