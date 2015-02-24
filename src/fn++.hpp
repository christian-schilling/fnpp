#ifndef _1af7690b_7fca_4464_9e14_c71bc5a29ee9
#define _1af7690b_7fca_4464_9e14_c71bc5a29ee9


#include "fn/iterators.hpp"
#include "fn/optional.hpp"
#include "fn/synchronized.hpp"
#include "fn/element.hpp"

/*
 * Some macros to make optional<T> usable with complex types.
 * As soon as generic lambdas can be used (C++14), these won't be
 * necessary anymore, and should not be used from that point onwards.
 */
#ifndef _MSC_VER
#define FN_TYPENAME typename
#else
#define FN_TYPENAME
#endif

#define FN_OTYPE(X) FN_TYPENAME fn::fn_::remove_reference<decltype(X)>::T::Type
#define use_(X) X >>[&](FN_OTYPE(X)&
#define _as(X) X)
#define with_(X) use_(X)_as(X)


#endif
