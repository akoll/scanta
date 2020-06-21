default: a.out
debug: a.dbg

a.out: main.cpp
	clang++ -std=c++2a -O3 -o a.out -lSDL2 -I../.. -I../../../lib main.cpp

a.dbg: main.cpp
	clang++ -std=c++2a -O3 -g -o a.dbg -lSDL2 -I../.. -I../../../lib main.cpp

run: default
	./a.out
