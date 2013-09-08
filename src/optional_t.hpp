template<typename _T>
class Optional
{
public:
    typedef _T T;
    Optional(Optional const&) = delete;
    inline Optional(Optional const&& original):
        value(original.value),
        has_value(original.has_value){}

    inline Optional(): has_value(false){}
    inline Optional(T const& value): value(value), has_value(true){}

    inline T const& default_to(T const& fallback) const
    {
        return use(
            [&](T const& v)->T const& {return v;},
            [&]()->T const& {return fallback;}
        );
    }

    template<typename ValueF, typename EmptyF>
    inline auto use(
        ValueF const& handle_value,
        EmptyF const& handle_no_value) const
        ->decltype(handle_value(*reinterpret_cast<T*>(0)))
    {
        return has_value ?
                    handle_value(const_cast<T const&>(value))
                  : handle_no_value();
    }

private:
    T value;
    bool const has_value;
};
