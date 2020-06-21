default: a.out
debug: a.dbg

a.out: main.cpp *.hpp */*.hpp */*/*.hpp
	clang++ -std=c++2a -O3 -o a.out -lSDL2 main.cpp

a.dbg: main.cpp *.hpp */*.hpp */*/*.hpp
	clang++ -std=c++2a -O3 -g -o a.dbg -lSDL2 main.cpp

run: default
	./a.out
