#ifndef _33128b50_cfb2_4f58_aa60_82c8585f832e
#define _33128b50_cfb2_4f58_aa60_82c8585f832e

#include "common.hpp"
#include "optional.hpp"

namespace std { class mutex; }

namespace fn {

template<typename Mutex>
void lock(Mutex& m) { m.lock(); }

template<typename Mutex>
void unlock(Mutex& m) { m.unlock(); }

template<typename T, typename Mutex> class guard;

/* template<typename T1, typename T2, typename Mutex> */
/* auto guard_cast(guard<T1&,Mutex> o, optional<T2> v) */
/*     ->guard<T2,Mutex> */
/* { */
/*     return guard<T2,Mutex>(fn_::move(o.mutex),fn_::move(v)); */
/* } */

template<typename T1, typename T2, typename Mutex>
auto guard_cast(guard<T1&,Mutex> o, optional<T2&> v)
    ->guard<T2&,Mutex>
{
    return guard<T2&,Mutex>(fn_::move(o.mutex),fn_::move(v));
}

template<typename T1, typename Mutex>
auto guard_cast(guard<T1&,Mutex> o, optional<void> v)
    ->guard<void,Mutex>
{
    return guard<void,Mutex>(fn_::move(o.mutex),fn_::move(v));
}

template<typename T, typename Mutex>
class guard
{
    optional<Mutex&> mutex;
    optional<T&> value;
public:
    guard(Mutex& mutex_, T& value_):
        mutex(mutex_),
        value(value_)
    {
        mutex >> lock<Mutex>;
    }

    guard(guard const&) = delete;

    guard(guard&& o)
    {
        value = o.value;
        mutex = o.mutex;
        o.value = {};
        o.mutex = {};
    }

    template<typename T1, typename T2, typename M>
    friend auto guard_cast(guard<T1&,M> o, optional<T2&> v)
        ->guard<T2&,M>;

    /* template<typename T1, typename T2, typename M> */
    /* friend auto guard_cast(guard<T1&,M> o, optional<T2> v) */
    /*     ->guard<T2,M>; */

    template<typename T1, typename M>
    friend auto guard_cast(guard<T1&,M> o, optional<void> v)
        ->guard<void,M>;

private:
    guard(optional<Mutex&>&& m, optional<T&>&& v)
    {
        value = fn_::move(v);
        mutex = fn_::move(m);
        v = {};
        m = {};
    }

public:
    template<typename F>
    auto operator>>(F const& f)
        -> decltype(guard_cast(fn_::move(*this),value >> f))
    {
        auto tmp = value >> f;
        auto r = guard_cast(fn_::move(*this),fn_::move(tmp));
        // Doing this without the temporary does not work: (compiler bug?)
        // auto r = guard_cast(fn_::move(*this),value >> f);
        mutex = {};
        value = {};
        return r;
    }

    ~guard() { mutex >> unlock<Mutex>; }
};



template<typename Mutex>
class guard<void,Mutex>
{
    optional<Mutex&> mutex;
public:
    guard(optional<Mutex&>&& m, optional<void>&&)
    {
        mutex = fn_::move(m);
        m = {};
    }

    ~guard() { mutex >> unlock<Mutex>; }
};

template<typename T,typename Mutex>
class synchronized_guard
{
    Mutex& mutex;
    T& value;
public:
    synchronized_guard(Mutex& mutex, T& value):
        mutex(mutex),
        value(value)
    {
        mutex.lock();
    }

    ~synchronized_guard() { mutex.unlock(); }
    T& operator*() { return value; }
    T* operator->() { return &value; }
};

template<typename T, typename Mutex=std::mutex>
class synchronized final
{
    Mutex mutable mutex;
    T value;
public:
    using Type = T;

    template<typename ...Args>
    synchronized(Args... args): value(args...) {}

    template<typename F>
    auto operator>>(F const& f) -> decltype(fn::guard<T&,Mutex>(mutex,value) >> f)
    {
        return fn::guard<T&,Mutex>(mutex,value) >> f;
    }

    template<typename F>
    auto operator>>(F const& f) const -> decltype(fn::guard<T const&,Mutex>(mutex,value) >> f)
    {
        return fn::guard<T const&,Mutex>(mutex,value) >> f;
    }

    T take()
    {
        mutex.lock();
        auto t = T(fn_::move(value));
        mutex.unlock();
        return fn_::move(t);
    }

    T clone()
    {
        mutex.lock();
        auto t = T(value);
        mutex.unlock();
        return fn_::move(t);
    }


    fn::synchronized_guard<T,Mutex> guard()
    {
        return fn::synchronized_guard<T,Mutex>(mutex,value);
    }

    fn::synchronized_guard<T const,Mutex> guard() const
    {
        return fn::synchronized_guard<T const,Mutex>(mutex,value);
    }
};

/* #define FN_FAIL A_functor_applied_to_a_guard_must_return_either_a_reference_or_void */

/* struct FN_FAIL { FN_FAIL() = delete; }; */

/* template<typename T, typename Mutex, typename F> */
/* FN_FAIL operator>>(synchronized<T,Mutex>,F) */
/* { */
/*     return {}; */
/* } */

/* #undef FN_FAIL */

};


#endif
