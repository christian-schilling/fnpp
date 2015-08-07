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

template<typename T1, typename T2, typename Mutex>
auto guard_cast(guard<T1,Mutex> o, optional<T2> v)
    ->guard<T2,Mutex>
{
    return guard<T2,Mutex>(fn_::move(o.mutex),fn_::move(v));
}

template<typename T, typename Mutex>
class guard
{
    optional<Mutex&> mutex;
    optional<T> value;
public:
    guard(Mutex& mutex_, T value_):
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

    /* template<typename T1> */
    /* friend class guard; */

    template<typename T1, typename T2, typename M>
    /* friend guard<T2,Mutex> guard_cast() */
    friend auto guard_cast(guard<T1,M> o, optional<T2> v)
        ->guard<T2,M>;

    guard(optional<Mutex&>&& m, optional<T>&& v)
    {
        value = fn_::move(v);
        mutex = fn_::move(m);
        v = {};
        m = {};
    }

    template<typename F>
    auto operator>>(F f)
        -> decltype(guard_cast(fn_::move(*this),value >> f))
    //decltype(guard_cast(*this,value >> f))
    {
        auto r = guard_cast(fn_::move(*this),value >> f);
        mutex = {};
        value = {};
        return r;
        /* return value >> f; */
        /* return guard_cast(*this,value >> f); */
    }

    /* template<typename F> */
    /* auto operator>>(F f) const -> guard<decltype(f(value)),Mutex> */
    /* { */
    /*     return guard<decltype(f(value)),Mutex>(value >> f); */
    /* } */

    ~guard() { mutex >> unlock<Mutex>; }
    /* optional<T&> operator*() { return value; } */
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
    auto operator>>(F f) -> decltype(fn::guard<T&,Mutex>(mutex,value) >> f)
    {
        return fn::guard<T&,Mutex>(mutex,value) >> f;
    }

    template<typename F>
    auto operator>>(F f) const -> decltype(fn::guard<T const&,Mutex>(mutex,value) >> f)
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


    /* fn::guard<T,Mutex> guard() */
    /* { */
    /*     return fn::guard<T,Mutex>(mutex,value); */
    /* } */

    /* fn::guard<T const,Mutex> guard() const */
    /* { */
    /*     return fn::guard<T const,Mutex>(mutex,value); */
    /* } */
};

};


#endif
