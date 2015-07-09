#ifndef _ad03a310_3d97_4841_a98d_7b7a65870185
#define _ad03a310_3d97_4841_a98d_7b7a65870185

#include "optional.hpp"

namespace fn {

namespace fn_ {

struct chained
{
    chained(): prev(this), next(this) {}

    chained(chained& o):
        prev(&o),
        next(o.next)
    {
        prev->next = this;
        next->prev = this;
    }

    chained(chained&& o):
        prev(o.prev),
        next(o.next)
    {
        prev->next = this;
        next->prev = this;
        o.prev = &o;
        o.next = &o;
    }

    ~chained()
    {
        prev->next = next;
        next->prev = prev;
    }

    template<typename F>
    void for_all(F f)
    {
        auto cur = this;
        do {
            f(*cur);
            cur = cur->next;
        } while(cur != this);
    }

    size_t count()
    {
        size_t c = 0;
        for_all([&](chained&){ ++c; });
        return c;
    }

    chained* prev;
    chained* next;
};

}

template<typename T> struct ref;

template<typename T>
struct shared
{
    explicit shared(T const& value): value(value) { }

    shared(shared&& o): value(fn_::move(o.value)), refs(fn_::move(o.refs))
    {
        refs >>[this](ref<T>& refs) {
            refs.retarget(this->value);
        };
        o.refs = {};
    }

    shared(shared const&) = delete;
    shared& operator=(shared const&) = delete;

    shared& operator=(T const& v)
    {
        value = v;
        return *this;
    }

    ~shared()
    {
        refs >>[this](ref<T>& refs) {
            refs.invalidate();
        };
    }

    operator T& () { return value; }

private:
    T value;

    friend struct ref<T>;
    optional<ref<T>&> refs;
};

template<typename T>
struct ref : public fn_::chained, public optional<T&>
{
    ref() {}
    ref(shared<T>& o): optional<T&>(o.value)
    {
        o.refs = *this;
    }

private:
    friend struct shared<T>;
    void retarget(T& value)
    {
        for_all([&](fn_::chained& x){
            auto& r = static_cast<ref&>(x);
            static_cast<optional<T&>&>(r) = value;
        });
    }

    void invalidate()
    {
        for_all([](fn_::chained& x){
            auto& r = static_cast<ref&>(x);
            static_cast<optional<T&>&>(r) = {};
        });
    }
};

}

#endif
