#ifndef _4fac367e_60d1_4c71_8690_910625d21ebd
#define _4fac367e_60d1_4c71_8690_910625d21ebd

#include <mutex>
#include <condition_variable>
#include <memory>

namespace fn{

namespace fn_ {

template<typename T>
struct Ring
{
    std::mutex mutex;
    std::condition_variable cv;
    size_t const size;

    size_t write_pos = 0;
    size_t read_pos = 0;

    T* data()
    {
        return reinterpret_cast<T*>(this+1);
    }

    Ring(Ring const&) = delete;

    Ring(size_t const size):
        size(size+1)
    {}

    Ring operator==(Ring&& o)
    {
        T* tmp;
        while((tmp = o.pop())){
            new (push()) T(fn_::move(*tmp));
            tmp->~T();
        }
    };

    bool empty() const { return write_pos == read_pos; }
    bool full() const { return (write_pos+1) % size == read_pos; }

    T* push()
    {
        if(!full()){
            auto const p = write_pos;
            write_pos = (write_pos + 1) % size;
            return data()+p;
        }
        return nullptr;
    }

    T* pop()
    {
        if(!empty()){
            auto const p = read_pos;
            read_pos = (read_pos + 1) % size;
            return data()+p;
        }
        return nullptr;
    }

    template<typename F>
    void remove_if(F f)
    {
        auto from = read_pos;
        auto to = read_pos;

        while(from < write_pos){
            if(from != to){
                data()[to].~T();
                new (data()+to) T(fn_::move(data()[from]));
                data()[from].~T();
            }
            if(!f(data()[from])){
                to = (to + 1) % size;
            }
            from = (from + 1) % size;
        }
        write_pos = to;
    }
};

}

template<typename T>
struct Channel
{
    class Send;

    class Receive
    {
        friend struct Channel;

        Receive(std::shared_ptr<uint8_t> queue_mem):
            queue_mem(queue_mem)
        {}

        std::shared_ptr<uint8_t> queue_mem;

        fn_::Ring<T>& queue()
        {
            return *reinterpret_cast<fn_::Ring<T>*>(queue_mem.get());
        }

    public:

        Receive(Receive const&) = delete;

        Receive(Receive&& o):
            queue_mem(fn_::move(o.queue_mem))
        {
            o.queue_mem = nullptr;
        }

        optional<T> operator()(uint64_t const timeout_ms)
        {
            if(!queue_mem){ return {}; }
            std::unique_lock<std::mutex> lock(queue().mutex);

            if(queue().empty()){
                queue().cv.wait_for(lock,std::chrono::milliseconds(timeout_ms));
            }

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
            std::lock_guard<std::mutex> quard(queue().mutex);
            queue().remove_if(f);
        }

    };

    class Send
    {
        friend struct Channel;

        std::shared_ptr<uint8_t> queue_mem;

        fn_::Ring<T>& queue()
        {
            return *reinterpret_cast<fn_::Ring<T>*>(queue_mem.get());
        }

        Send(std::shared_ptr<uint8_t> queue_mem):
            queue_mem(queue_mem)
        {}

    public:

        Send(Send const& o):
            queue_mem(o.queue_mem)
        {}

        bool operator()(T v)
        {
            if(!queue_mem){ return false; }
            queue().mutex.lock();

            auto const p = queue().push();
            if(p){
                new (p) T(fn_::move(v));
                queue().mutex.unlock();
                queue().cv.notify_one();
                return true;
            }
            queue().mutex.unlock();
            return false;
        }
    };

    Channel(size_t const size):
        queue_mem(
            new uint8_t[sizeof(fn_::Ring<T>)+sizeof(T)*(size+1)],
            []( uint8_t *p ) {
                reinterpret_cast<fn_::Ring<T>*>(p)->~Ring<T>();
                delete[] p;
            }
        ),
        receive(queue_mem),
        send(queue_mem)
    {
        new (queue_mem.get()) fn_::Ring<T>(size);
    }

    std::shared_ptr<uint8_t> queue_mem;
    Receive receive;
    Send send;
};

}

#endif
