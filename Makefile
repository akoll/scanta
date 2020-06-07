default: ecs

ecs: ecs.cpp *.hpp **/*.hpp
	clang++ -std=c++2a -O3 -o ecs ecs.cpp

run: ecs
	./ecs
