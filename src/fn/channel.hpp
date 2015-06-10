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

        Receiver() {}

        optional<Sender&> tx;
        std::shared_ptr<std::mutex> mutex;

    public:

        Receiver(Receiver const&) = delete;

        Receiver(Receiver&& o):
            mutex(fn_::move(o.mutex))
        {
            if(!mutex){ return; }
            auto guard = fn_::guard(*mutex);

            tx = fn_::move(o.tx);
            queue = fn_::move(o.queue);

            tx >>[&](Sender& tx){
                tx.rx = *this;
            };
        }


        optional<T> recv()
        {
            if(!mutex){ return {}; }
            auto guard = fn_::guard(*mutex);

            auto const p = queue.pop();
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
            if(!mutex){ return; }
            auto guard = fn_::guard(*mutex);
            queue.remove_if(f);
        }

        fn_::Ring<T,N> queue;
    };

    class Sender
    {
        friend struct Channel;

        optional<Receiver&> rx;
        std::shared_ptr<std::mutex> mutex;

        Sender(Receiver& rx):
            rx(rx)
        {
            rx.tx = *this;
        }

    public:

        Sender(Sender const&) = delete;

        Sender(Sender&& o):
            mutex(fn_::move(o.mutex))
        {
            if(!mutex){ return; }
            auto guard = fn_::guard(*mutex);

            rx = fn_::move(o.rx);
            o.rx = {};
        }

        bool send(T v)
        {
            if(!mutex){ return false; }
            auto guard = fn_::guard(*mutex);

            return rx >>[&](Receiver& rx){
                auto const p = rx.queue.push();
                if(p){
                    new (p) T(fn_::move(v));
                    return true;
                }
                return false;
            } | false;
        }
    };

    Channel():
        tx(rx)
    {
        tx.mutex = rx.mutex = std::make_shared<std::mutex>();
    }

    Receiver rx;
    Sender tx;
};

}

#endif
