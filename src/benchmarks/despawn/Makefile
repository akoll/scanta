NAME = despawn
CC = clang++
CFLAGS = -Wall -Wdocumentation -std=c++2a -O3 -fopenmp -I../.. -I../../../lib -lSDL2

default: bench_$(NAME).pdf

tov.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DTUPLE_OF_VECTORS

heap.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DHEAP

heap_set.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DHEAP -DHEAP_SET

heap_smart.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DHEAP_SMART

-include *.d

graph.tex: tov.out heap.out heap_smart.out heap_set.out heap_smart_set.out
	./heap_smart.out | python ../bench2tex.py green!75!black -64 320000 > graph.tex
	./heap.out | python ../bench2tex.py blue -64 320000 >> graph.tex
	./heap_set.out | python ../bench2tex.py blue!75!black -64 320000 >> graph.tex
	./tov.out | python ../bench2tex.py orange -64 320000 >> graph.tex

bench_$(NAME).pdf: $(NAME).tex graph.tex
	pdflatex --jobname=bench_$(NAME) $(NAME).tex

clean:
	rm -rf *.out *.d *.log *.aux bench_$(NAME).pdf

cleantex: clean
	rm -rf graph.tex

.PHONY: default run clean
