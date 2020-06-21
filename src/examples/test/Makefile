CC = clang++
CFLAGS = -Wall -std=c++2a -O3 -fopenmp -I../.. -I../../../lib -lSDL2

%.out: %.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS)

-include *.d

default: main.out
run: main.out
	./main.out
clean:
	rm -rf *.out *.d

.PHONY: default run clean
