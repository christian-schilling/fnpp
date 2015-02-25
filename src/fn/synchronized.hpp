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
};

};


#endif
