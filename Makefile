default: ecs

a.out: main.cpp *.hpp **/*.hpp
	clang++ -std=c++2a -O3 -o a.out main.cpp

run: a.out
	./a.out
