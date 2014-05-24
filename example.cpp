/*
 *  Usage example for optional<T> with C++1y.
 *
 *  As C++1y will support generic lambdas, we don't need
 *  macros anymore to use optional<T> in a compact way.
 *
 *  compile with:
 *  clang++ -o example -std=c++1y example.cpp -I./src
 */

#define _GLIBCXX_HAVE_GETS // Hack arround bug in libstdc++ with -std=c++1y
void gets(){}              //

#include <string>
#include <vector>
#include <iostream>

#include "fn++.hpp"
using namespace fn;


optional<std::string> maybe_hello(int i)
{
    if(i==1){
        return {"hello"};
    }
    else{
        return {};
    }
}

optional<int> readInt(){ return {}; }


int main(int,char**)
{
    readInt() // did I get a real int ?
    >>[](int i){ // Yes
        std::cout << "my int is: " << i << std::endl;   // use my int
    }
    >>[]{ // No
        std::cout << "I have no int" << std::endl;
    };

    for(auto const i: range(5)){
        std::cout << i << " ";

        auto const msg = maybe_hello(i)
        >>[](auto greeting){
            return greeting + " world!";
        }
        >>[]{
            return std::string{"no hello"};
        };

        std::cout << msg << std::endl;
    }
    return 0;
}
