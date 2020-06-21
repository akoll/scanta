CC = clang++
CFLAGS = -std=c++2a -O3 -I../.. -I../../../lib -lSDL2

%.out: %.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS)

-include *.d

default: main.out
run: main.out
	./main.out
clean:
	rm -rf *.out *.d

.PHONY: default run clean