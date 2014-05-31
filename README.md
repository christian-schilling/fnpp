fn++
====

A header that provides some helpers to enhance functional style programming in C++.

[![Build Status](https://travis-ci.org/initcrash/fnpp.svg?branch=master)](https://travis-ci.org/initcrash/fnpp)

Features
--------
* iterator factories
    - range
    - map
    - filter
    - zip
    - enumerate
* optional<T>: like the proposed std::optional<T>, but better.
* element(...).of(...) / element(...).in(...): range checked access to
  containers, returning optional<T> objects.
* zero dependencies

Usage
-----

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
