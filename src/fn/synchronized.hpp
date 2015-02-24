#ifndef _33128b50_cfb2_4f58_aa60_82c8585f832e
#define _33128b50_cfb2_4f58_aa60_82c8585f832e

namespace fn {

template<typename T, typename Mutex>
class synchronized final
{
    Mutex mutex;
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
};

};


#endif
