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

maybe_int()
>>[](int i){
    std::cout << "my int is: " << i << std::endl;
}
||[]{
    std::cout << "no int here" << std::endl;
};
```

In addition to avoiding undefined behavior, the whole thing has the added advantage
of beeing an expression that can have a value:
```C++
auto text = maybe_int()
>>[](int i){
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

###Element

The element function creates optionals while accessing containers:
```C++
std::vector<int> v = {1,2,3};
optional<int&> i = element(1).of(v); // i refs 2
i = element(5).of(v);                // i refs nothing
i = element(-1).of(v);               // i refs 3

std::map<std::string,int> m = {{"one":1},{"two":2}}
i = element("one").in(m);            // i refs 1
i = element("three").in(m);          // i refs nothing
```

When used with a container of optionals the tilde shotcut becomes very usefull:
```C++
std::vector<optional<int>> v = {1,3,5,{},4};
auto o = element(2).of(v); // Type of o is optional<optional<int&>>
auto i = ~element(2).of(v);// Type of i is optional<int&>
```

Additionally it allows easy iteration over parts of a container:
```C++
std::vector<int> v = {1,2,3,4,5};
for(auto& i: element(2).to_last().of(v)){
    // i refs 3,4,5
}
for(auto& i: element(2).to(4).of(v)){
    // i refs 2,3
}
for(auto& i: element(-4).to(-1).of(v)){
    // i refs 2,3,4
}
for(auto& i: element(1).to(-1).of(v).by(2)){
    // i refs 1,3
}
for(auto& i: element(3).to(1).of(v)){
    // no iterations
}
```
