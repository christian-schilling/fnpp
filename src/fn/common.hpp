#ifndef _33744f5c_1401_479e_857c_a098aa160a2e
#define _33744f5c_1401_479e_857c_a098aa160a2e

namespace fn{
namespace fn_{

/*
 *  remove reference from a type, similar to std::remove_reference
 */
template<class R> struct remove_reference      {typedef R T;};
template<class R> struct remove_reference<R&>  {typedef R T;};
template<class R> struct remove_reference<R&&> {typedef R T;};


/*
 *  cast to rvalue reference, similar to std::move
 */
template <class T>
typename remove_reference<T>::T&&
move(T&& a)
{
    return ((typename remove_reference<T>::T&&)a);
}

/*
 *  This helper allows for easier usage of a functions return type
 *  in a type deduction context.
 *  Additionally it works around a strange msvc internal compiler error.
 */
template<class F, class T>
static auto return_type(F f,T&& value)->decltype(f(value))
{
    return f(value);
}


}}

#endif
