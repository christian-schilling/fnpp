#ifndef _33128b50_cfb2_4f58_aa60_82c8585f832e
#define _33128b50_cfb2_4f58_aa60_82c8585f832e

#include "common.hpp"

namespace fn {

template<typename T, typename Mutex>
class synchronized final
{
    Mutex mutable mutex;
    T value;
public:
    using Type = T;

    template<typename ...Args>
    synchronized(Args... args): value(args...) {}

    template<typename F>
    void operator>>(F const& f)
    {
        mutex.lock();
        f(value);
        mutex.unlock();
    }

    template<typename F>
    void operator>>(F const& f) const
    {
        mutex.lock();
        f(value);
        mutex.unlock();
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

    class synchronized_guard_const
    {
        Mutex& mutex;
        T const& value;
    public:
        synchronized_guard_const(Mutex& mutex, T const& value):
            mutex(mutex),
            value(value)
        {
            mutex.lock();
        }

        ~synchronized_guard_const() { mutex.unlock(); }
        T const& operator*() const { return value; }
        T const* operator->() const { return &value; }
    };

    synchronized_guard guard()
    {
        return synchronized_guard(mutex,value);
    }

    synchronized_guard_const guard() const
    {
        return synchronized_guard_const(mutex,value);
    }
};

};


#endif
