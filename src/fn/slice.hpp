#ifndef _83c9a570_2b4b_481b_a810_85d3b5f3fe91
#define _83c9a570_2b4b_481b_a810_85d3b5f3fe91

#include "optional.hpp"
#include "iterators.hpp"

namespace fn{

template<typename slice>
class slice_IT
{
    slice& s;
    size_t position;
public:

    slice_IT(slice& s, size_t position): s(s), position(position) {}

    bool operator!=(slice_IT const& other) const
    {
        return position != other.position;
    }

    slice_IT const& operator++()
    {
        position += 1;
        return *this;
    }

    typename slice::value_type& operator*()
    {
        return s._data[position];
    }
};

template<typename T, size_t S=0>
class slice
{
public:
    using value_type = T;

private:
    T* _data;

    template<typename O_T, size_t O_S>
    friend class slice;

    friend class slice_IT<slice>;

    explicit slice(T* const d,int):
        _data(d)
    {}

public:

    template<size_t OS>
    slice(T(&d)[OS]):
        slice(d,0)
    {
        static_assert(S <= OS,"fn::slice: source array to small");
    }

    static slice from_pointer(T* const d)
    {
        return slice(d,0);
    }

    template<typename O_T, size_t O_S>
    slice(slice<O_T,O_S>& other):
        slice(other._data,0)
    {
        static_assert(S <= O_S,"fn::slice: target to small");
    }

    slice& operator=(slice const& o)
    {
        _data = o._data;
        return *this;
    }

    template<size_t O>
    auto offset() const -> slice<T,S-O>
    {
        static_assert(O < S,"fn::slice: offset to large");
        return slice<T,S-O>(_data + O,0);
    }

    auto offset(size_t o) const -> slice<T,0>
    {
        return slice<T,0>(_data + o,(o<size()) ? size()-o : 0);
    }

    template<size_t I>
    T& at()
    {
        static_assert(I < S,"fn::slice: index out of bounds");
        return _data[I];
    }

    template<size_t I>
    T const& at() const
    {
        static_assert(I < S,"fn::slice: index out of bounds");
        return _data[I];
    }


    size_t size() const
    {
        return S;
    }

    optional<T&> operator[](size_t const i)
    {
        if(i<S){
            return _data[i];
        }
        else{
            return {};
        }
    }

    optional<T const&> operator[](size_t const i) const
    {
        if(i<S){
            return _data[i];
        }
        else{
            return {};
        }
    }

    slice_IT<slice> begin()
    {
        return {*this,0};
    }

    slice_IT<slice> end()
    {
        return {*this,size()};
    }

    slice& fill(T const& v)
    {
        for(auto& x: *this){
            x = v;
        }
        return *this;
    }

    slice& copy(slice const& o)
    {
        for(auto&& i: range(S)){
            _data[i] = o._data[i];
        }
        return *this;
    }

    template<typename RT, size_t C=0>
    auto reinterpret_as()
        -> decltype(slice<RT,(C==0) ? (S*sizeof(T))/sizeof(RT) : C>::from_pointer(
            reinterpret_cast<RT*>(_data)
        ))
    {
        static_assert(
            ((C==0) ? (S*sizeof(T))/sizeof(RT) : C) * sizeof(RT)
            <=
            sizeof(T) * S,
            "fn::slice: reinterpret does not fit"
        );

        return slice<RT,(C==0) ? (S*sizeof(T))/sizeof(RT) : C>::from_pointer(
            reinterpret_cast<RT*>(_data)
        );
    }
};

template<typename T>
class slice<T,0>
{
public:
    using value_type = T;

private:
    size_t _size;
    T* _data;
    friend class slice_IT<slice>;

public:
    slice(T* const _data, size_t const _size):
        _size(_size),
        _data(_data)
    {}

    template<typename O_T>
    slice(slice<O_T,0>& s):
        slice(s._data,s.size())
    {}

    template<typename O_T, size_t O_S>
    slice(slice<O_T,O_S>& s):
        slice(s._data,s.size())
    {}

    template<typename V>
    slice(V& v):
        slice(v.data(),v.size())
    {}

    slice& operator=(slice const& o)
    {
        _data = o._data;
        _size = o._size;
        return *this;
    }

    optional<T&> operator[](size_t const i)
    {
        if(i<_size){
            return _data[i];
        }
        else{
            return {};
        }
    }

    optional<T const&> operator[](size_t const i) const
    {
        if(i<_size){
            return _data[i];
        }
        else{
            return {};
        }
    }

    size_t size() const { return _size; }


    slice_IT<slice> begin()
    {
        return {*this,0};
    }

    slice_IT<slice> end()
    {
        return {*this,size()};
    }

    slice& fill(T const& v)
    {
        for(auto&& x: *this){
            x = v;
        }
        return *this;
    }

    slice& copy(slice const& o)
    {
        for(auto&& i: range(size() < o.size() ? size() : o.size())){
            _data[i] = o._data[i];
        }
        return *this;
    }

    auto offset(size_t o) const -> slice<T,0>
    {
        return slice<T,0>(_data + o,(o<size()) ? size()-o : 0);
    }

    template<size_t S>
    auto subslice() -> optional<slice<T,S>>
    {
        if(S > size()){
            return {};
        }
        else{
            return slice<T,S>::from_pointer(_data);
        }
    }

    slice subslice(size_t const s)
    {
        if(s > size()){
            return slice(nullptr,0);
        }
        else{
            return slice(_data,s);
        }
    }

    template<typename RT>
    auto reinterpret_as(size_t const c=0) -> slice<RT>
    {
        auto const new_size = ((c==0) ? (size()*sizeof(T))/sizeof(RT) : c);
        return slice<RT>(
            reinterpret_cast<RT*>(_data),
            new_size*sizeof(RT) <= size()*sizeof(T) ? new_size : 0
        );
    }
};

template<typename V>
auto make_slice(V& v)
    ->slice<typename V::value_type>
{
    return {v};
}

template<size_t S, typename V>
auto slice_from_pointer(V* v)
    ->slice<V,S>
{
    return slice<V,S>::from_pointer(v);
}




}


#endif
