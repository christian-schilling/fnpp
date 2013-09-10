namespace funny{

namespace _ {

template<typename C>
struct noconst{ typedef C T; };
template<typename B>
struct noconst<B const&>{ typedef B T; };

template<typename T>
struct IsTrue{ bool operator()(T i)const {return static_cast<bool>(i);} };


template<typename T>
class Range
{
    class IT
    {
    public:
        IT(T const& position): position(position) {}
        IT(T const& position, T const& step): IT(position),step(step) {}
        bool operator!=(IT const& other)const {return position!=other.position;}
        IT const& operator++() {position+=step;return *this;}
        T const& operator*()const {return position;}
    private:
        T position;
        T step = 1;
    };

    IT from;
    IT const to;
public:
    Range(T const& to): from(0), to(to) {}
    IT const& begin()const { return from; }
    IT const& end()const { return to; }
};};

template<typename T,typename ...Args>
auto range(T const& t,Args const& ...args) -> _::Range<T>{
    return _::Range<T>(t,args...);
}

namespace _ {
template<typename FN,typename G, typename OtherIT>
class Map
{
    class IT
    {
    private:
        FN const& fn;
        OtherIT other_it;
    public:
        IT(FN fn,OtherIT other_it): fn(fn), other_it(other_it) {}
        bool operator!=(IT const& other)const {return other_it!=other.other_it;}
        IT const& operator++() {++other_it;return *this;}
        auto operator*()const -> decltype(fn(*other_it)) {return fn(*other_it);}
    };

    IT const from;
    IT const to;

public:
    Map(FN fn, G const& g): from(fn,g.begin()), to(fn,g.end()) {}
    IT const& begin() const { return from; }
    IT const& end() const { return to; }
};};

template<typename FN,typename G>
auto map(FN const& fn,G const& g) -> _::Map<FN,G,
    typename _::noconst<decltype(g.begin())>::T>{
    return _::Map<FN,G,typename _::noconst<decltype(g.begin())>::T>(fn,g);
}

namespace _ {
template<typename FN,typename G, typename OtherIT>
class Filter
{
    class IT
    {
    private:
        FN const& fn;
        OtherIT other_it;
    public:
        IT(FN fn,OtherIT other_it): fn(fn), other_it(other_it){}
        bool operator!=(IT const& other){
            while(!fn(*other_it) and other_it!=other.other_it){ ++other_it; }
            return other_it!=other.other_it;
        }
        IT const& operator++() { ++other_it; return *this; }
        auto operator*()const -> decltype(*other_it) { return *other_it; }
    };

    IT const from;
    IT const to;

public:
    Filter(FN fn, G const& g): from(fn,g.begin()), to(fn,g.end()) {}
    IT const& begin() const { return from; }
    IT const& end() const { return to; }
};};

template<typename FN,typename G>
auto filter(FN const& fn,G const& g) -> _::Filter<FN,G,
    typename _::noconst<decltype(g.begin())>::T>
{
    return _::Filter<FN,G,typename _::noconst<decltype(g.begin())>::T>(fn,g);
}

template<typename G>
auto filter(G const& g)
     -> decltype(filter(_::IsTrue<decltype(*g.begin())>{},(*g.begin(),g)))
{
    return filter(_::IsTrue<decltype(*g.begin())>{},g);
}

template<typename T>
class optional
{
public:
    typedef T Type;
    optional(optional const&) = delete;
    inline optional(optional const&& original):
        has_value(original.has_value),
        value(original.value){}

    inline optional(): has_value(false){}
    inline optional(T const& value): has_value(true), value(value){}

    inline T const& operator||(T const& fallback) const
    {
        return use(
            [](T const& v)->T const& {return v;},
            [&]()->T const& {return fallback;}
        );
    }

    template<typename ValueF>
    inline optional const& use(
        ValueF const& handle_value) const
    {
        if(has_value) {handle_value(const_cast<T const&>(value));}
        return *this;
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

    bool const has_value;
private:
    T value;
};

};
