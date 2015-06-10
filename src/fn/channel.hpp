#ifndef _4fac367e_60d1_4c71_8690_910625d21ebd
#define _4fac367e_60d1_4c71_8690_910625d21ebd

#include <mutex>
#include <memory>

namespace fn{

namespace fn_ {

template<typename T, size_t N_>
class Ring
{
    auto static const N = N_ + 1;

    size_t p0 = 0;
    size_t p1 = 0;

    struct T_mem { uint8_t mem[sizeof(T)]; };
    T_mem data[N];

public:

    std::mutex mutex;

    Ring(Ring const&) = delete;

    Ring() {}

    Ring operator==(Ring&& o)
    {
        T* tmp;
        while((tmp = o.pop())){
            new (push()) T(fn_::move(*tmp));
            tmp->~T();
        }
    };

    bool empty() const { return p0 == p1; }
    bool full() const { return (p0+1) % N == p1; }

    T* push()
    {
        if(!full()){
            auto const p = p0;
            p0 = (p0 + 1) % N;
            return reinterpret_cast<T*>(data+p);
        }
        return nullptr;
    }

    T* pop()
    {
        if(!empty()){
            auto const p = p1;
            p1 = (p1 + 1) % N;
            return reinterpret_cast<T*>(data+p);
        }
        return nullptr;
    }

    template<typename F>
    void remove_if(F f)
    {
        auto from = p1;
        auto to = p1;

        while(from < p0){
            if(from != to){
                reinterpret_cast<T*>(data+to)->~T();
                new (data+to) T(fn_::move(*reinterpret_cast<T*>(data+from)));
                reinterpret_cast<T*>(data+from)->~T();
            }
            if(!f(*reinterpret_cast<T*>(data+from))){
                to = (to + 1) % N;
            }
            from = (from + 1) % N;
        }
        p0 = to;
    }
};

template<typename Mutex>
struct Guard
{
    Mutex& mutex;
    Guard(Mutex& mutex): mutex(mutex) { mutex.lock(); }
    ~Guard() { mutex.unlock(); }
};

template<typename Mutex>
auto guard(Mutex& mutex) -> Guard<Mutex> { return {mutex}; }

}

template<typename T, size_t N>
struct Channel
{
    class Sender;

    class Receiver
    {
        friend struct Channel;

        Receiver(std::shared_ptr<uint8_t> queue_mem):
            queue_mem(queue_mem)
        {}

        optional<Sender&> tx;

        std::shared_ptr<uint8_t> queue_mem;

        fn_::Ring<T,N>& queue()
        {
            return *reinterpret_cast<fn_::Ring<T,N>*>(queue_mem.get());
        }

    public:

        Receiver(Receiver const&) = delete;

        Receiver(Receiver&& o):
            queue_mem(fn_::move(o.queue_mem))
        {
            o.queue_mem = nullptr;
        }


        optional<T> recv()
        {
            if(!queue_mem){ return {}; }
            auto guard = fn_::guard(queue().mutex);

            auto const p = queue().pop();
            if(p){
                optional<T> tmp(fn_::move(*p));
                p->~T();
                return tmp;
            }
            return {};
        }

        template<typename F>
        void remove_if(F f)
        {
            if(!queue_mem){ return; }
            auto guard = fn_::guard(queue().mutex);
            queue().remove_if(f);
        }

    };

    class Sender
    {
        friend struct Channel;

        std::shared_ptr<uint8_t> queue_mem;

        fn_::Ring<T,N>& queue()
        {
            return *reinterpret_cast<fn_::Ring<T,N>*>(queue_mem.get());
        }

        Sender(std::shared_ptr<uint8_t> queue_mem):
            queue_mem(queue_mem)
        {}

    public:

        Sender(Sender const& o):
            queue_mem(o.queue_mem)
        {}

        bool send(T v)
        {
            if(!queue_mem){ return false; }
            auto guard = fn_::guard(queue().mutex);

            auto const p = queue().push();
            if(p){
                new (p) T(fn_::move(v));
                return true;
            }
            return false;
        }
    };

    Channel():
        queue_mem( new uint8_t[sizeof(fn_::Ring<T,N>)], []( uint8_t *p ) { delete[] p; } ),
        rx(queue_mem),
        tx(queue_mem)
    {
        new (queue_mem.get()) fn_::Ring<T,N>;
    }

    std::shared_ptr<uint8_t> queue_mem;
    Receiver rx;
    Sender tx;
};

}

#endif
