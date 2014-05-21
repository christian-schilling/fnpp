#ifndef _1af7690b_7fca_4464_9e14_c71bc5a29ee9
#define _1af7690b_7fca_4464_9e14_c71bc5a29ee9

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
        typedef decltype(fn(*other_it)) value_type;
        IT(FN fn,OtherIT other_it): fn(fn), other_it(other_it) {}
        bool operator!=(IT& other){return other_it!=other.other_it;}
        IT const& operator++() {++other_it;return *this;}
        auto operator*()const -> value_type {return fn(*other_it);}
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
        typedef typename OtherIT::value_type value_type;

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

template<typename O, typename ValueF, typename T>
struct optional_helper
{
    O o;
    T value;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#endif
    optional_helper(O const& o,typename O::Type& o_value, ValueF const& handle_value):
        o(o),
        value(o.has_value
                ? handle_value(const_cast<typename O::Type&>(o_value))
                : *&value)
    {}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    template<typename EmptyF>
    auto operator>>(EmptyF const& handle_no_value) const
        ->decltype(handle_no_value())
    {
        return o.has_value
            ? value
            : handle_no_value();
    }
};

template<typename O, typename ValueF>
struct optional_helper<O,ValueF,void>
{
    O o;

    optional_helper(O const& o,typename O::Type& o_value, ValueF const& handle_value):
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
struct optional_helper<O,void,void>
{
    O o;

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

}

template<typename T>
class optional
{
    friend class optional<typename fn_::noconst<T>::T&>;
    friend class optional<typename fn_::noconst<T>::T>;
    friend class optional<typename fn_::noconst<
        typename fn_::remove_reference<T>::T
    >::T const&>;
    friend class optional<typename fn_::noconst<
        typename fn_::remove_reference<T>::T
    >::T&>;
    friend class optional<T const&>;
    friend class optional<T const>;
    friend class optional<T&>;

public:
    bool const has_value;
private:
    T value;

public:
    typedef T Type;

    template<class O>
    inline optional(optional<O> const& original):
        has_value(original.has_value),
        value(original.value){}

    inline optional(optional const& original):
        has_value(original.has_value),
        value(original.value){}

    inline optional(optional const&& original):
        has_value(original.has_value),
        value(original.value){}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#endif
    inline optional():
        has_value(false),
        value(*&value)
    {}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    inline optional(T const& value): has_value(true), value(value){}

    inline T operator||(T fallback) const
    {
        return has_value ? value : fallback;
    }

    inline bool operator==(optional const& other) const
    {
        if(has_value && other.has_value){
            return value == other.value;
        }
        return false;
    }

    typename fn_::remove_reference<T>::T operator--(int)
    {
        if(has_value){
            return value;
        }
        else{
            return {};
        }
    }

    inline bool operator!=(optional const& other) const
    {
        return !(*this == other);
    }

    template<typename ValueF>
    inline auto operator>>(
        ValueF const& handle_value)
    ->fn_::optional_helper<
        optional,
        ValueF,
        decltype(handle_value(const_cast<T&>(value)))
    >
    {
        return {*this, value, handle_value};
    }

    inline auto operator!()
        ->fn_::optional_helper<optional,void,void>
    {
        return {*this};
    }
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
