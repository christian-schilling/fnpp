#ifndef _1af7690b_7fca_4464_9e14_c71bc5a29ee9
#define _1af7690b_7fca_4464_9e14_c71bc5a29ee9

namespace fn{

namespace _ {

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
auto range(T const& t,Args const& ...args) -> _::Range<T>{
    return _::Range<T>(t,args...);
}

namespace _ {
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
auto map(FN const& fn,G const& g) -> _::Map<FN,G,
    typename _::noconst<decltype(g.begin())>::T>{
    return _::Map<FN,G,typename _::noconst<decltype(g.begin())>::T>(fn,g);
}

namespace _ {
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

namespace _ {
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

template<template<typename,typename> class PairT=_::Pair, typename A,typename B>
auto zip(A const& a,B const& b) -> _::Zip<PairT,A,B,
        typename _::noconst<decltype(a.begin())>::T,
        typename _::noconst<decltype(b.begin())>::T
    >{
    return _::Zip<PairT,A,B,
        typename _::noconst<decltype(a.begin())>::T,
        typename _::noconst<decltype(b.begin())>::T
    >(a,b);
}

template<typename A>
auto enumerate(A const& a)
     ->decltype(zip(range(-1),a))
{
    return zip(range(-1),a);
}

namespace _ {
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
auto filter(FN const& fn,G const& g) -> _::Filter<FN,G,
    typename _::noconst<decltype(g.begin())>::T>
{
    return _::Filter<FN,G,typename _::noconst<decltype(g.begin())>::T>(fn,g);
}

template<typename G>
auto filter(G const& g)
     -> decltype(filter(_::IsTrue<decltype(*g.begin())>{},(*g.begin(),g)))
{
    return filter(_::IsTrue<decltype(*g.begin())>{},g);
}


template<typename T>
class optional
{
public:
    typedef T Type;
    optional(optional const&) = delete;
    inline optional(optional const&& original):
        has_value(original.has_value),
        value(original.value){}

    inline optional(): has_value(false){}
    inline optional(T const& value): has_value(true), value(value){}

    inline T const& operator||(T const& fallback) const
    {
        return (*this)(
            [](T const& v)->T const& {return v;},
            [&]()->T const& {return fallback;}
        );
    }

    template<typename ValueF>
    inline optional const& operator()(
        ValueF const& handle_value) const
    {
#ifndef _MSC_VER
        static_assert(_::is_void<
            decltype(handle_value(const_cast<T const&>(value)))>() == true,
            "this function must not have a return value"
        );
#endif
        if(has_value) {handle_value(const_cast<T const&>(value));}
        return *this;
    }

    template<typename ValueF, typename EmptyF>
    inline auto operator()(
        ValueF const& handle_value,
        EmptyF const& handle_no_value) const
        ->decltype(handle_value(*reinterpret_cast<T*>(0)))
    {
        return has_value ?
                    handle_value(const_cast<T const&>(value))
                  : handle_no_value();
    }

    explicit operator bool() const { return has_value; };

private:
    bool const has_value;
    T value;
};

};

#endif
