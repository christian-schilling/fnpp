#ifndef _951b3841_da8b_4067_a4a9_3fdd5450e8ad
#define _951b3841_da8b_4067_a4a9_3fdd5450e8ad

namespace fn{

namespace fn_{

/*
 *  remove const from a type
 */
template<typename C> struct noconst{ typedef C T; };
template<typename C> struct noconst<C const&>{ typedef C T; };
template<typename C> struct noconst<C const>{ typedef C T; };

/*
 * Iterator factory for iteration over ranges of values
 */
template<typename T>
class Range
{
    class IT
    {
    public:
        typedef T value_type;

        IT(T const& position):
            position(position)
        {}

        IT(T const& position, T const& step):
            position(position),
            step(step)
        {}

        bool operator!=(IT const& other) const
        {
            return position != other.position;
        }

        IT const& operator++()
        {
            position += step;
            return *this;
        }

        T const& operator*() const
        {
            return position;
        }

    private:
        T position;
        T step = 1;
    };

    IT from;
    IT const to;

public:
    Range(T const& to):
        from(0),
        to(to)
    {}

    Range(T const& from, T const& to):
        from(from),
        to(to)
    {}

    Range(T const& from, T const& to, T const& step):
        from(from,step),
        // choose an end that will not get skipped when
        // advancing by 'step'
        to(((to-from)%step) ? to + (step-(to-from)%step) : to,step)
    {}

    IT const& begin() const
    {
        return from;
    }

    IT const& end() const
    {
        return to;
    }
};


/*
 * Factory to create an iterator that applies a function to every
 * element of an other iterator.
 */
template<typename FN,typename G, typename OtherIT>
class Map
{
    class IT
    {
    private:
        FN const& fn;
        OtherIT other_it;

    public:
        IT(FN fn,OtherIT other_it):
            fn(fn),
            other_it(other_it)
        {}

        bool operator!=(IT& other)
        {
            return other_it != other.other_it;
        }

        IT const& operator++()
        {
            ++other_it;
            return *this;
        }

        auto operator*() const -> decltype(fn(*other_it))
        {
            return fn(*other_it);
        }
    };

    IT const from;
    IT const to;

public:
    Map(FN fn, G const& g):
        from(fn,g.begin()),
        to(fn,g.end())
    {}

    IT const& begin() const
    {
        return from;
    }

    IT const& end() const
    {
        return to;
    }
};

/*
 * Makes begin/end iterator pairs usable with range based for
 */
template<typename OtherIT>
class AsRange
{
    OtherIT const from;
    OtherIT const to;

public:
    AsRange(OtherIT from, OtherIT to):
        from(from),
        to(to)
    {}

    OtherIT const& begin() const
    {
        return from;
    }

    OtherIT const& end() const
    {
        return to;
    }
};

/*
 * Creates an iterator that allows simultaneous iteration
 * over two other iterators.
 * The iteration will terminate if either of the two input
 * iterators end.
 */
template<
    template<typename,typename> class PairT,
    typename A,
    typename B,
    typename OtherIT1,
    typename OtherIT2
>
class Zip
{
    class IT
    {
    private:
        typename fn_::noconst<OtherIT1>::T other_it1;
        typename fn_::noconst<OtherIT2>::T other_it2;
        typedef PairT<decltype(*other_it1),decltype(*other_it2)> Pair;

    public:
        typedef Pair value_type;

        IT(OtherIT1 other_it1,OtherIT2 other_it2):
            other_it1(other_it1),other_it2(other_it2)
        {}

        bool operator!=(IT& other)
        {
            return other_it1!=other.other_it1
                && other_it2 != other.other_it2;
        }

        IT& operator++()
        {
            ++other_it1;
            ++other_it2;
            return *this;
        }

        Pair operator*() {
            return Pair(*other_it1,*other_it2);
        }
    };

    IT const from;
    IT const to;

public:
    Zip(A& a, B& b):
        from(a.begin(),b.begin()),
        to(a.end(),b.end())
    {}

    IT const& begin() const
    {
        return from;
    }

    IT const& end() const
    {
        return to;
    }
};


/*
 * The value type used by Zip.
 * It aliases the first and second element to a few
 * other names that make more sense depending on context.
 */
template<typename A, typename B>
struct Pair
{
    Pair(A& a, B& b):
        first(a),second(b),
        key(a),value(b),
        nr(a),item(b)
    {}

    A& first;
    B& second;

    A& key;
    B& value;

    A& nr;
    B& item;
};

/*
 * Creates an iterator that only iterates over only those
 * elements of another iterator, that match a condition supplied
 * as a function.
 */
template<typename FN,typename G, typename OtherIT>
class Filter
{
    class IT
    {
    private:
        FN const& fn;
        OtherIT other_it;
    public:
        IT(FN fn,OtherIT other_it):
            fn(fn),
            other_it(other_it)
        {}

        bool operator!=(IT& other)
        {
            while(other_it!=other.other_it && !fn(*other_it)){
                ++other_it;
            }
            return other_it != other.other_it;
        }

        IT const& operator++()
        {
            ++other_it;
            return *this;
        }

        auto operator*() const -> decltype(*other_it)
        {
            return *other_it;
        }
    };

    IT const from;
    IT const to;

public:
    Filter(FN fn, G const& g):
        from(fn,g.begin()),
        to(fn,g.end())
    {}

    IT const& begin() const
    {
        return from;
    }

    IT const& end() const
    {
        return to;
    }
};

template<typename T>
struct IsTrue
{
    bool operator()(T i) const
    {
        return !!i;
    }
};

}

/*
 * The following functions allow instantiation of the above classes,
 * without explicitly specifying the template arguments.
 * This is necessary, because function templates support
 * automatic type deduction by parameter type, while classes do not.
 */
template<typename T,typename ...Args>
auto range(T const& t,Args const& ...args) -> fn_::Range<T>
{
    return fn_::Range<T>(t,args...);
}

template<typename FN,typename G>
auto map(FN const& fn,G const& g)
    ->fn_::Map<FN,G,typename fn_::noconst<decltype(g.begin())>::T>
{
    return fn_::Map<FN,G,typename fn_::noconst<decltype(g.begin())>::T>(fn,g);
}

template<typename OtherIT>
auto as_range(OtherIT const& b, OtherIT const& e) -> fn_::AsRange<OtherIT>
{
    return fn_::AsRange<OtherIT>(b,e);
}

template<
    template<typename,typename> class PairT=fn_::Pair,
    typename A,
    typename B
>
auto zip(A&& a,B&& b)
    -> fn_::Zip<PairT,A,B, decltype(a.begin()), decltype(b.begin())>
{
    return fn_::Zip<PairT,A,B, decltype(a.begin()), decltype(b.begin())>(a,b);
}

template<typename A>
auto enumerate(A&& a) -> decltype(zip(range(-1),a))
{
    return zip(range(-1),a);
}

template<typename FN,typename G>
auto filter(FN const& fn,G const& g)
    ->fn_::Filter<FN,G, typename fn_::noconst<decltype(g.begin())>::T>
{
    return fn_::Filter<FN,G,typename fn_::noconst<decltype(g.begin())>::T>(fn,g);
}

template<typename G>
auto filter(G const& g)
    ->decltype(filter(fn_::IsTrue<decltype(*g.begin())>{},(*g.begin(),g)))
{
    return filter(fn_::IsTrue<decltype(*g.begin())>{},g);
}

/*
 * Reduction is so simple, it does not need a helper class
 */
template<typename I,typename T, typename F>
T reduce(I const& iter, T const& neutral, F const& f)
{
    T v = neutral;
    for(auto const& x: iter){
        v = f(v,x);
    }
    return v;
}

}

#endif
