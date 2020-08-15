CC = clang++
CFLAGS = -Wall -std=c++2a -O3 -fopenmp -I../.. -I../../../lib -lSDL2

%.out: %.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS)

-include *.d

default: main.out
run: main.out
	./main.out
graph: main.out
	./main.out | python bench2tex.py
clean:
	rm -rf *.out *.d


.PHONY: default run clean
