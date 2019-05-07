#ifndef include_guard_158beae5_5030_4848_8eb7_579443f81a17
#define include_guard_158beae5_5030_4848_8eb7_579443f81a17


#include <array>
#include <fn/slice.hpp>

namespace fn
{
template <class, class>
struct TL;

template <class T, class N = TL<void, void> >
struct TL
{
    typedef T type;
    typedef N next;
};

template <>
struct TL<void, void>
{
    typedef void type;
    typedef void next;
};

template <class L, class T, size_t ID = 0U>
struct TL_id
{
    typedef TL_id<typename L::next, T, ID + 1U> recurse;
    typedef T type;
    enum V
    {
        // clang-format off
        value = std::is_same<
            typename std::remove_const<typename L::type>::type,
            typename std::remove_const<T>::type
        >::value
            ? ID
            : static_cast<size_t>(recurse::value),

        value_const = std::is_same<typename L::type,T>::value
            ? ID
            : static_cast<size_t>(recurse::value_const)
        // clang-format on
    };
};

template <typename T, bool SAME_ID>
struct actual_T;

template <typename T>
struct actual_T<T, true>
{
    typedef T type;
};

template <typename T>
struct actual_T<T, false>
{
    typedef const T type;
};

template <typename TL_ID>
struct return_T
{
    typedef typename actual_T<typename TL_ID::type, TL_ID::value == TL_ID::value_const>::type type;
};

template <class T, size_t ID>
struct TL_id<void, T, ID>
{
    enum
    {
        value       = 0xFFFF,
        value_const = 0xFFFF
    };
};

template <class L>
struct TL_id<L, void>
{
    enum
    {
        value       = 0xFFFF,
        value_const = 0xFFFF
    };
};

template <class T>
struct size_of
{
    enum V
    {
        value = sizeof(T)
    };
};

template <>
struct size_of<void>
{
    enum
    {
        value = 0
    };
};

template <class L, class = typename L::type>
struct TL_count
{
    typedef typename L::type T;
    typedef TL_count<typename L::next> recurse;
    enum V
    {
        value = 1 + recurse::value
    };
};

template <class L>
struct TL_count<L, void>
{
    enum
    {
        value = 0
    };
};

template <typename T, typename MS>
struct multi_to
{
    static slice<T> sl(const MS& ms) { return ms.template get<T>(); }
};

template <typename MS>
struct multi_to<void, MS>
{
    static slice<void> sl(const MS&) { return {}; }
};

/**
 * Maintain a set of equally sized slices
 */
template <
    class T0, // We should always have at least two types
    class T1, // Assumed by implementation in several places
    class T2 = void,
    class T3 = void,
    class T4 = void,
    class T5 = void,
    class T6 = void,
    class T7 = void>
class multi_slice
{
public:
    typedef TL<T0, TL<T1, TL<T2, TL<T3, TL<T4, TL<T5, TL<T6, TL<T7> > > > > > > > types;

private:
    // Ensure type uniqueness
    static_assert((TL_id<types, T0>::value) >= 0, "types not unique");
    static_assert((TL_id<types, T1>::value) >= 1, "types not unique");
    static_assert((TL_id<types, T2>::value) >= 2, "types not unique");
    static_assert((TL_id<types, T3>::value) >= 3, "types not unique");
    static_assert((TL_id<types, T4>::value) >= 4, "types not unique");
    static_assert((TL_id<types, T5>::value) >= 5, "types not unique");
    static_assert((TL_id<types, T6>::value) >= 6, "types not unique");
    static_assert((TL_id<types, T7>::value) >= 7, "types not unique");

    size_t _size;
    std::array<const void*, TL_count<types>::value> _p;

public:
    ~multi_slice()
    { /* special thanks to our favorite compiler vendor */
    }

    multi_slice() : _size(0) {}

    template <
        typename O0,
        typename O1,
        typename O2,
        typename O3,
        typename O4,
        typename O5,
        typename O6,
        typename O7>
    multi_slice(const multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>& other)
    {
        multi_slice tmp(
            multi_to<T0, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> >::sl(other),
            multi_to<T1, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> >::sl(other),
            multi_to<T2, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> >::sl(other),
            multi_to<T3, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> >::sl(other),
            multi_to<T4, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> >::sl(other),
            multi_to<T5, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> >::sl(other),
            multi_to<T6, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> >::sl(other),
            multi_to<T7, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> >::sl(other));
        _size = tmp._size;
        _p    = tmp._p;
    }

    multi_slice(const multi_slice& other) : _size(other._size), _p(other._p) {}

    multi_slice(
        slice<T0> s0,
        slice<T1> s1,
        slice<T2> s2 = slice<T2>{},
        slice<T3> s3 = slice<T3>{},
        slice<T4> s4 = slice<T4>{},
        slice<T5> s5 = slice<T5>{},
        slice<T6> s6 = slice<T6>{},
        slice<T7> s7 = slice<T7>{})
    : _size(0)
    {
        const size_t size  = s0.size();
        const size_t count = TL_count<types>::value;

        // clang-format off
                       { if (size != s0.size()) { return; } _p[0] = s0.data(); }
                       { if (size != s1.size()) { return; } _p[1] = s1.data(); }
        if (count > 2) { if (size != s2.size()) { return; } _p[2] = s2.data(); }
        if (count > 3) { if (size != s3.size()) { return; } _p[3] = s3.data(); }
        if (count > 4) { if (size != s4.size()) { return; } _p[4] = s4.data(); }
        if (count > 5) { if (size != s5.size()) { return; } _p[5] = s5.data(); }
        if (count > 6) { if (size != s6.size()) { return; } _p[6] = s6.data(); }
        if (count > 7) { if (size != s7.size()) { return; } _p[7] = s7.data(); }
        // clang-format on

        _size = size;
    }

    template <typename T>
    size_t idx(T* v) const
    {
        return v - data<T>();
    }

    size_t size() const { return _size; }

    template <class T>
    slice<typename return_T<TL_id<types, T> >::type> get() const
    {
        return slice<typename return_T<TL_id<types, T> >::type>(data<T>(), _size);
    }

    template <class T>
    typename return_T<TL_id<types, T> >::type* data() const
    {
        // ensure T is in the list
        static_assert(
            (static_cast<size_t>(TL_id<types, T>::value)
             < static_cast<size_t>(TL_count<types>::value)), "T not in the multi_slice");

        return reinterpret_cast<typename return_T<TL_id<types, T> >::type*>(
            const_cast<void*>(_p[TL_id<types, T>::value]));
    }

    template <class T>
    typename return_T<TL_id<types, T> >::type& at(const size_t idx) const
    {
        return data<T>()[idx];
    }

    multi_slice& trim(const size_t limit)
    {
        _size = (limit <= size()) ? limit : 0U;
        return *this;
    }

    multi_slice& advance(size_t offset)
    {
        const size_t by = offset >= _size ? _size : offset;
        _size -= by;

        const size_t c = TL_count<types>::value;

        // clang-format off
                   { _p[0] = reinterpret_cast<const uint8_t*>(_p[0]) + by * size_of<T0>::value; }
                   { _p[1] = reinterpret_cast<const uint8_t*>(_p[1]) + by * size_of<T1>::value; }
        if (c > 2) { _p[2] = reinterpret_cast<const uint8_t*>(_p[2]) + by * size_of<T2>::value; }
        if (c > 3) { _p[3] = reinterpret_cast<const uint8_t*>(_p[3]) + by * size_of<T3>::value; }
        if (c > 4) { _p[4] = reinterpret_cast<const uint8_t*>(_p[4]) + by * size_of<T4>::value; }
        if (c > 5) { _p[5] = reinterpret_cast<const uint8_t*>(_p[5]) + by * size_of<T5>::value; }
        if (c > 6) { _p[6] = reinterpret_cast<const uint8_t*>(_p[6]) + by * size_of<T6>::value; }
        if (c > 7) { _p[7] = reinterpret_cast<const uint8_t*>(_p[7]) + by * size_of<T7>::value; }
        // clang-format on
        return *this;
    }
};

} // namespace fn



#endif /* include_guard */
