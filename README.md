#fn++

A header that provides some helpers to enhance functional style programming in C++.

[![Build Status](https://travis-ci.org/initcrash/fnpp.svg?branch=master)](https://travis-ci.org/initcrash/fnpp)

##Features
* Iterator factories
    - range
    - map
    - filter
    - zip
    - enumerate
* optional<T>: like the proposed std::optional<T>, but better.
* element(...).of(...) / element(...).in(...): range checked access to
  containers, returning optional<T> objects.
* Zero dependencies
* Tested with gcc 4.8, clang 3.4 and msvc++ 2013

##Usage

###Iterator factories
The iterator factories make range based for loops more expressive:
```C++
for(auto i: range(10)){
    std::cout << i << std::endl;
}

std::vector<std::string> v = {"a","b","c"};
for(auto i: enumerate(v)){
    std::cout << i.nr << ":" << i.item << std::endl;
}
```

###Optional
The optional class resembles the proposed c++17 std::optional, but in contrast
to the std version, it does not support dereferencing and therefore never
results in undefined behavior. Instead values can be accessed using the >> and
|| operators in conjunction with callables:
```C++
optional<int> maybe_int();

maybe_int() >>[](int i){
    std::cout << "my int is: " << i << std::endl;
}
||[]{
    std::cout << "no int here" << std::endl;
};
```

In addition to avoiding undefined behavior, the whole thing has the added advantage
of beeing an expression that can have a value:
```C++
auto text = maybe_int() >>[](int i){
    return std::string{"my int is: "} + std::to_string(i);
}
||[]{
    return std::string{"no int here"};
};
```

The >> operator actually returns an optional object itself, making it chainable:
```C++
auto text = maybe_int()
>>[](int i)->double{
    return i/2.0;
}
>>[](double d){
    return std::string{"half of my int is: "} + std::to_string(d);
}
||[]{
    return std::string{"no int here"};
};
```

Functions in the chain can short circuit execution:
```C++
auto text = maybe_int()
>>[](int i)->optional<int>{
    if(i < 10){ return i; }
    else { return {}; }
}
>>[](int i){
    return std::string{"my small int is: "} + std::to_string(i);
}
| std::string{"no small int here"};
```

The bar | is a shortcut for defaulting to a value:
```C++
auto int_or_minus_one = maybe_int() | -1; // default to -1 if no int
```

If T is default constructible there is another shortcut:
```C++
auto int_or_0 = ~maybe_int() // same as: maybe_int() | 0
```
