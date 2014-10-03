#ifndef _58924b14_de27_45db_9ac4_64f1ee7e92f7
#define _58924b14_de27_45db_9ac4_64f1ee7e92f7

#include "fn/optional.hpp"

namespace fn{

namespace fn_{

template<class T>
class Element
{
    T const i;

    template<typename C>
    class IT
    {
        C& container;
        unsigned int position;
        int step = 1;

    public:
        typedef T value_type;
        IT(C& container, T const& position):
            container(container),
            position(position)
        {}

        IT(IT const& o, int step):
            container(o.container),
            position(o.position),
            step(step)
        {}

        bool operator!=(IT const& other) const
        {
            return position < other.position;
        }

        IT const& operator++()
        {
            position += step;
            return *this;
        }

        auto operator*() const
            ->decltype(container[position])
        {
            return container[position];
        }
    };

    template<class Container>
    class Range
    {
        IT<Container> from;
        IT<Container> const to;
        int step = 1;

    public:
        Range(Container& c, T const& from, T const& to):
            from(c,from),
            to(c,to)
        {}

        Range(Range const& o, IT<Container> from):
            from(from),
            to(o.to),
            step(o.step)
        {}

        IT<Container> const& begin() const { return from; }
        IT<Container> const& end() const { return to; }

        Range by(int step)
        {
            return Range(*this,IT<Container>(from,step));
        }
    };

    class UpperLimit
    {
        Element from;
        Element to{0};
    public:
        UpperLimit(Element const from, Element const to):
            from(from),
            to(to)
        {}

        UpperLimit(Element const from):
            from(from)
        {}

        template<class Container>
        auto of(Container& c) const
            ->Range<Container>
        {
            auto const size = c.size();
            auto index = from.get_index(size);
            if(index > size){
                index = 0;
            }

            auto limit = to.i ? to.get_index(size) : size;
            if(limit > size){
                limit = size;
            }

            if(limit < index){
                limit = index = 0;
            }

            return Range<Container>(c,index,limit);
        }
    };

public:
    Element(T const i): i{i} {}

    template<typename S>
    S get_index(S const& size) const
    {
        return static_cast<S>(i<0?(size+i):i);
    }

    template<class Container>
    auto of(Container& c) const
        ->optional<decltype(c[0])&>
    {
        auto const size = c.size();
        auto index = get_index(size);

        if(index < size){
            return c[index];
        }
        else{
            return {};
        }

        (void)c.back(); // protect against using with std::map
    }

    UpperLimit to_last()
    {
        return UpperLimit(i);
    }

    UpperLimit to(Element limit)
    {
        return UpperLimit(i,limit);
    }


    template<class Container>
    auto in(Container& c) const
        ->optional<decltype(c.at(i))>
    {
        if(c.count(i)){
            return c.at(i);
        }
        else{
            return {};
        }
    }
};

}

template<class T>
fn_::Element<T> element(T const i)
{
    return fn_::Element<T>(i);
}

}


#endif
