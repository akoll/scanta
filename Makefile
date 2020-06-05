default: ecs

ecs: ecs.cpp *.hpp **/*.hpp
	g++ -std=c++2a -fconcepts -O3 -o ecs ecs.cpp

run: ecs
	./ecs
