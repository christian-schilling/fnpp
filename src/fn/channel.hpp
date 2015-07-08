#ifndef _4fac367e_60d1_4c71_8690_910625d21ebd
#define _4fac367e_60d1_4c71_8690_910625d21ebd

#include <mutex>
#include <condition_variable>
#include <memory>
#include "fn++.hpp"

namespace fn{

namespace fn_ {

template<typename T>
class Ring
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    size_t const size;

    size_t write_pos = 0;
    size_t read_pos = 0;

    Ring(Ring const&) = delete;

    static std::shared_ptr<Ring> create(size_t const size)
    {
        auto p = std::shared_ptr<Ring>(
            reinterpret_cast<Ring*>(
                new uint8_t[sizeof(Ring) + sizeof(T)*(size+1)]
            ),
            [](Ring *p) {
                p->~Ring();
                delete[] reinterpret_cast<uint8_t*>(p);
            }
        );
        new (p.get()) Ring(size+1);
        return p;
    }

    bool empty() const { return write_pos == read_pos; }
    bool full() const { return (write_pos+1) % size == read_pos; }

    T* push()
    {
        if(!full()){
            auto const ret = data()+write_pos;
            write_pos = (write_pos + 1) % size;
            return ret;
        }
        return nullptr;
    }

    T* pop()
    {
        if(!empty()){
            auto const ret = data()+read_pos;
            read_pos = (read_pos + 1) % size;
            return ret;
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
            }
            if(!f(data()[to])){ to = (to + 1) % size; }
            from = (from + 1) % size;
        }
        write_pos = to;

        while(to < from) {
            data()[to].~T();
            to = (to + 1) % size;
        }
    }

private:
    Ring(size_t const size):
        size(size)
    {}

    T* data() { return reinterpret_cast<T*>(this+1); }
};

}

template<typename T>
class Channel
{
    using Ring = fn_::Ring<T>;

public:
    class Send;

    class Receive
    {
        friend class Channel;

        Receive(std::shared_ptr<Ring> queue):
            queue(queue)
        {}

        std::shared_ptr<Ring> queue;

    public:

        Receive(Receive const&) = delete;

        Receive(Receive&& o):
            queue(fn_::move(o.queue))
        {
            o.queue = nullptr;
        }

        optional<T> operator()(uint64_t const timeout_ms)
        {
            if(!queue){ return {}; }
            std::unique_lock<std::mutex> lock(queue->mutex);

            if(queue->empty()){
                queue->cv.wait_for(lock,std::chrono::milliseconds(timeout_ms));
            }

            auto const p = queue->pop();
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
            if(!queue){ return; }
            std::lock_guard<std::mutex> quard(queue->mutex);
            queue->remove_if(f);
        }

    };

    class Send
    {
        friend class Channel;

        std::shared_ptr<Ring> queue;


        Send(std::shared_ptr<Ring> queue):
            queue(queue)
        {}

    public:

        Send(Send const& o):
            queue(o.queue)
        {}

        bool operator()(T v)
        {
            if(!queue){ return false; }
            queue->mutex.lock();

            auto const p = queue->push();
            if(p){
                new (p) T(fn_::move(v));
                queue->mutex.unlock();
                queue->cv.notify_one();
                return true;
            }
            queue->mutex.unlock();
            return false;
        }
    };

    Channel(size_t const size):
        queue(Ring::create(size)),
        receive(queue),
        send(queue)
    {
    }

    std::shared_ptr<Ring> queue;
    Receive receive;
    Send send;
};

}

#endif
