#ifndef _83c9a570_2b4b_481b_a810_85d3b5f3fe91
#define _83c9a570_2b4b_481b_a810_85d3b5f3fe91

#include <array>
#include "iterators.hpp"
#include "optional.hpp"

namespace fn{

template<typename slice, typename T>
class slice_IT
{
    slice& s;
    size_t position;
public:

    using value_type = T;

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

    T& operator*()
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

    friend class slice_IT<slice,T>;
    friend class slice_IT<slice const,T>;

    explicit slice(T* const d,int):
        _data(d)
    {}

public:

    template<size_t OS>
    slice(T(&d)[OS]):
        slice(d,0)
    {
        static_assert(S <= OS, "fn::slice: source array to small");
    }

    template<typename OT, size_t OS>
    slice(std::array<OT,OS>& array):
        slice(array.data(),0)
    {
        static_assert(
            sizeof(T) == sizeof(OT) && S <= OS,
            "fn::slice: source std::array to small"
        );
    }

    static slice from_pointer(T* const d)
    {
        return slice(d,0);
    }

    template<size_t O_S>
    slice(slice<T,O_S> const& other):
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
    auto subslice() const -> slice<T,S-O>
    {
        static_assert(O < S,"fn::slice: offset to large");
        return slice<T,S-O>(_data + O,0);
    }

    auto subslice(size_t o) const -> slice<T,0>
    {
        return slice<T,0>(_data + o,(o<size()) ? size()-o : 0);
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

    optional<T&> operator[](size_t const i) const
    {
        if(i<S){
            return _data[i];
        }
        else{
            return {};
        }
    }

    slice_IT<slice const,T> begin() const
    {
        return {*this,0};
    }

    slice_IT<slice const,T> end() const
    {
        return {*this,size()};
    }

    slice const& fill(T const& v) const
    {
        for(auto& x: *this){
            x = v;
        }
        return *this;
    }

    void copy(slice const& o) const
    {
        for(auto&& i: range(S)){
            _data[i] = o._data[i];
        }
    }

    template<typename RT, size_t C=0>
    auto reinterpret_as() const
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
    friend class slice_IT<slice,T>;
    friend class slice_IT<slice const,T>;
    friend class slice<T const,0>;

public:
    slice(): slice(nullptr,0)
    {}

    slice(T* const _data, size_t const _size):
        _size(_size),
        _data(_data)
    {}

    template<typename O_T>
    slice(slice<O_T,0> const& s):
        slice(s._data,s.size())
    {}

    template<size_t O_S>
    slice(slice<T,O_S> const& s):
        slice(s._data,s.size())
    {}

    template<size_t O_S>
    slice(slice<T,O_S>& s):
        slice(s._data,s.size())
    {}

    slice(slice const& o):
        slice(o._data,o.size())
    {}

    operator slice<T const>()
    {
        return slice<T const>(_data,_size);
    }

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

    optional<T&> operator[](size_t const i) const
    {
        if(i<_size){
            return _data[i];
        }
        else{
            return {};
        }
    }

    size_t size() const { return _size; }
    T* data() const { return _data; }

    slice_IT<slice const,T> begin() const
    {
        return {*this,0};
    }

    slice_IT<slice const,T> end() const
    {
        return {*this,size()};
    }

    slice const& fill(T const& v) const
    {
        for(auto&& x: *this){
            x = v;
        }
        return *this;
    }

    /*
     * Copies the data from source into this.
     * Returns the part of source that was not copied.
     */
    slice<T const> copy(slice<T const> const source) const
    {
        size_t n = 0;
        auto s = source.begin();
        for(auto&& d: *this){
            if(!(s != source.end())){ break; }
            d = *s;
            ++s;
            ++n;
        }
        return source.subslice(n);
    }

    bool operator==(slice const o) const
    {
        return _data == o._data && _size == o._size;
    }

    bool compare(slice const o) const
    {
        if(size() != o.size()){
            return false;
        }
        for(auto&& v: zip(*this,o)){
            if(!(v.first == v.second)){
                return false;
            }
        }
        return true;
    }

    auto subslice(size_t o) const -> slice<T,0>
    {
        return slice<T,0>(_data + o,(o<size()) ? size()-o : 0);
    }

    template<size_t S>
    auto first() -> optional<slice<T,S>>
    {
        if(S > size()){
            return {};
        }
        else{
            return slice<T,S>::from_pointer(_data);
        }
    }

    slice first(size_t const s) const
    {
        if(s > size()){
            return slice(nullptr,0);
        }
        else{
            return slice(_data,s);
        }
    }

    template<typename RT>
    auto reinterpret_as(size_t const c=0) const -> slice<RT>
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


template<>
class slice<void, 0> {
public:
    size_t size() const { return 0U; }
    void* data() const { return 0L; }
};



}


#endif
