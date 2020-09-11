CC = clang++
CFLAGS = -Wall -Wdocumentation -std=c++2a -O3 -fopenmp -I../.. -I../../../lib -lSDL2

default: bench_spiral.pdf

tov.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DTUPLE_OF_VECTORS

heap.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DHEAP

heap_smart.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DHEAP_SMART

-include *.d

graph.tex: tov.out heap.out heap_smart.out
	./heap_smart.out | python ../bench2tex.py green!75!black 32 > graph.tex
	./heap.out | python ../bench2tex.py blue 32 >> graph.tex
	./tov.out | python ../bench2tex.py orange 32 >> graph.tex

bench_spiral.pdf: spiral.tex graph.tex
	pdflatex --jobname=bench_spiral spiral.tex

clean:
	rm -rf *.out *.d *.log *.aux bench_spiral.pdf

cleantex: clean
	rm -rf graph.tex

.PHONY: default run clean
