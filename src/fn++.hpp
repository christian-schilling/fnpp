#ifndef _1af7690b_7fca_4464_9e14_c71bc5a29ee9
#define _1af7690b_7fca_4464_9e14_c71bc5a29ee9

#include <stdio.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif

#ifndef _MSC_VER
#define FN_TYPENAME typename
#else
#define FN_TYPENAME
#endif

namespace fn{

template<typename T> class optional;

namespace fn_ {

template<class F, class T>
static auto return_type(F f,T value)
->decltype(f(value))
{
    return f(value);
}

template<typename T>
struct return_cast {
    template<typename F,typename  V>
    static optional<T> func(F&& f,V&& v) {return f(v);}

    template<typename V>
    static T value(V&& v) {return v;}
};


template< class R > struct remove_reference      {typedef R T;};
template< class R > struct remove_reference<R&>  {typedef R T;};
template< class R > struct remove_reference<R&&> {typedef R T;};

template<typename C>
struct noconst{ typedef C T; };
template<typename B>
struct noconst<B const&>{ typedef B T; };

template<typename T>
struct IsTrue{ bool operator()(T i)const { return !!i; } };

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

template<typename T> class optional_ref;
template<typename T> class optional_value;

template<typename T>
class optional_base
{
public:
    typedef T Type;
    bool valid() const { return !!value; }

protected:
    optional_base(T* const p):
        value(p)
    {}

private:
    friend class optional<T const&>;
    friend class optional<T const>;
    friend class optional<T&>;
    friend class optional<T>;
    friend class optional_value<T>;
    friend class optional_ref<T>;

    T* value = nullptr;

public:
    template<typename F>
    T operator|(F fallback) const
    {
        return valid() ? *value : fallback;
    }

    bool operator==(T const& other_value) const
    {
        return valid() && (*value == other_value);
    }

    bool operator!=(T const& other_value) const
    {
        return !((*this) == other_value);
    }

    bool operator==(optional_base const& other) const
    {
        return (valid() && other.valid()) && (*value == *other.value);
    }

    bool operator!=(optional_base const& other) const
    {
        return !(*this == other);
    }

    template<typename ValueF>
    auto operator>>(
        ValueF const& handle_value) const
        ->decltype(
            return_cast<
                decltype(return_type(handle_value,*value))
            >::func(handle_value,*value)
        )
    {
        if(valid()){
            return return_cast<decltype(return_type(handle_value,*value))>::func(handle_value,*value);
        }
        else{
            return {};
        }
    }

    template<typename EmptyF>
    auto operator||(EmptyF const& handle_no_value) const
        ->decltype(handle_no_value())
    {
        if(!valid()){
            return handle_no_value();
        }
        else {
            return return_cast<decltype(handle_no_value())>::value(*value);
        }
    }

    T operator~() const
    {
        return (*this) | T{};
    }
};

template<typename T>
class optional_value : public optional_base<T>
{
protected:
    unsigned char value_mem[sizeof(T)];

    optional_value():
        optional_base<T>(nullptr)
    {}

    optional_value(T* v) :
        optional_base<T>(v)
    {}

    ~optional_value()
    {
        if(this->valid()){
            reinterpret_cast<T*>(value_mem)->~T();
        }
    }

public:
    optional_value& operator=(optional_value const& other)
    {
        if(other.value){
            *optional_base<T>::value = *other.value;
        }
        else{
            if(this->valid()){
                reinterpret_cast<T*>(value_mem)->~T();
            }
            optional_base<T>::value = nullptr;
        }
        return *this;
    }
};

template<typename T>
class optional_ref : public optional_base<T>
{
protected:
    optional_ref():
        optional_base<T>(nullptr)
    {}

    optional_ref(T* v):
        optional_base<T>(v)
    {}

    optional_ref(T&& value):
        optional_base<T>(true,value)
    {}

public:
    template<typename F>
    T& operator|(F& fallback) const
    {
        return (*this)
        >>[&](T& v)->T& { return v; }
        ||[&]()->T& { return fallback; };
    }

    template<typename F>
    T operator|(F const& fallback) const
    {
        return (*this)
        >>[&](T v)->T { return v; }
        ||[&]()->T { return fallback; };
    }

    optional_ref& operator=(optional_ref const& other)
    {
        optional_base<T>::value = other.value;
        return *this;
    }
};

}

template<typename T>
class optional final : public fn_::optional_value<T>
{
    using fn_::optional_value<T>::value_mem;

    friend class optional<T const>;
    friend class optional<T const&>;
    friend class optional<T&>;

public:
    optional():
        fn_::optional_value<T>()
    {}

    optional(T const& v):
        fn_::optional_value<T>(reinterpret_cast<T*>(value_mem))
    {
        new (value_mem) T{v};
    }

    optional(optional<T&> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T const&> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T const> const& original):
        fn_::optional_value<T>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }
};

template<typename T>
class optional<T const> final : public fn_::optional_value<T const>
{
    using fn_::optional_value<T const>::value_mem;

    friend class optional<T>;

public:
    optional():
        fn_::optional_value<T const>()
    {}

    optional(optional<T> const& original):
        fn_::optional_value<T const>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }

    optional(optional<T&> const& original):
        fn_::optional_value<T const>(original.valid()
            ? reinterpret_cast<T*>(value_mem)
            : nullptr)
    {
        original >>[&](T const& v){ new (value_mem) T{v};};
    }
};

template<typename T>
class optional<T&> final : public fn_::optional_ref<T>
{
    friend class optional<T const&>;
    friend class optional<T const>;
    friend class optional<T>;

public:
    optional():
        fn_::optional_ref<T>()
    {}

    optional(T& v):
        fn_::optional_ref<T>(&v)
    {}

    optional(optional<T> const& original):
        fn_::optional_ref<T>(original.value)
    {}
};

template<typename T>
class optional<T const&> final : public fn_::optional_ref<T const>
{
    friend class optional<T const>;
    friend class optional<T>;

public:
    optional():
        fn_::optional_ref<T const>()
    {}

    optional(T const& v):
        fn_::optional_ref<T const>(&v)
    {}

    optional(optional<T> const& original):
        fn_::optional_ref<T const>(original.value)
    {}

    optional(optional<T&> const& original):
        fn_::optional_ref<T const>(original.value)
    {}
};

template<>
class optional<void> final
{
    bool no_value;
public:
    optional(): no_value(true) {}
    optional(bool): no_value(false) {}

    template<typename EmptyF>
    void operator||(EmptyF const& handle_no_value) const
    {
        if(no_value){
            return handle_no_value();
        }
    }
};

namespace fn_ {

template<>
struct return_cast<void> {

    template<typename F,typename  V>
    static
    optional<void> func(F&& f,V&& v) {f(v); return true;}

    template<typename V>
    static void value(V&&) {}
};


template<typename T>
struct return_cast<optional<T>> {

    template<typename F,typename  V>
    static
    optional<T> func(F&& f,V&& v) {return f(v);}

    template<typename V>
    static T value(V&& v) {return v;}
};

template<class T>
class Element
{
    T const i;
public:
    Element(T const i): i{i} {}

    template<class Container>
    auto of(Container& c) const
        ->optional<decltype(c.at(0))&>
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
    auto in(Container& c) const
        ->optional<decltype(c.at(i))&>
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

#define FN_OTYPE(X) FN_TYPENAME fn::fn_::remove_reference<decltype(X)>::T::Type
#define use_(X) X >>[&](FN_OTYPE(X)&
#define _as(X) X)

#define with_(X) use_(X)_as(X)

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif
