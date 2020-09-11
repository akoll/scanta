CC = clang++
CFLAGS = -Wall -Wdocumentation -std=c++2a -O3 -fopenmp -I../.. -I../../../lib -lSDL2

default: bench_spawn.pdf

tov.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DTUPLE_OF_VECTORS

heap.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DHEAP

heap_smart.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DHEAP_SMART

-include *.d

graph.tex: tov.out heap.out heap_smart.out
	./heap_smart.out | python ../bench2tex.py green!75!black 128 > graph.tex
	./heap.out | python ../bench2tex.py blue 128 >> graph.tex
	./tov.out | python ../bench2tex.py orange 128 >> graph.tex

bench_spawn.pdf: graph.tex
	pdflatex --jobname=bench_spawn spawn.tex

clean:
	rm -rf *.out *.d *.log *.aux bench_spawn.pdf

cleantex: clean
	rm -rf graph.tex

.PHONY: default run clean
