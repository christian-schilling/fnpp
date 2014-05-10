fn++
====

A header that provides some helpers to enhance functional style programming in C++.

Features
--------
* iterators (those work on all iterators, not just containers, and are lazy evaluated)
    - range
    - map
    - filter
    - zip
    - enumerate
* optional<T>: represent a value that may or may not be present
* element(...).of(...) / element(...).in(...): range checked access to
  containers, returning optional<T> objects.
* zero dependencies (not even std::*)

Examples
--------
see test/tests.cpp

[![Build Status](https://travis-ci.org/initcrash/fnpp.svg?branch=master)](https://travis-ci.org/initcrash/fnpp)
