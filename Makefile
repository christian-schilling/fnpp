all : test
	./test

test : ./tests/test.cpp ./src/optional_t.hpp
	clang++ -std=c++11 -Wall -Wextra -Werror -DGTEST_LANG_CXX11 -I./src ./tests/test.cpp -lgtest -o test

