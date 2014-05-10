all : test example
	./test
	# ./example

test : ./tests/test.cpp ./src/fn++.hpp Makefile
	clang++ -O3 -g -std=c++11 -Wall -Wextra -Werror -Wno-unknown-pragmas -DGTEST_LANG_CXX11 -I./src ./tests/test.cpp -lgtest -lpthread -o test

# example : ./example.cpp ./src/fn++.hpp Makefile
# 	$(CXX) -O3 -g -std=c++11 -Wall -Wextra -Werror -DGTEST_LANG_CXX11 -I./src ./example.cpp -o example

