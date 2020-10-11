CC = g++
CFLAGS = -Wall -std=c++2a -O3 -fopenmp -I../.. -I../../../lib -I../../../lib/taskflow -I../../../lib/entt/src -lSDL2

default: bench_spawn.pdf

tov.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DCONTIGUOUS

tov_tf.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DCONTIGUOUS -DOUTER_PARALLEL

heap.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DSCATTERED

heap_set.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DSCATTERED -DHEAP_SET

heap_smart.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DSCATTERED -DHEAP_SMART

entt.out: main.cpp $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) -DENTT

-include *.d

graph.tex: tov.out tov_tf.out heap.out entt.out heap_set.out heap_smart.out
	./heap_smart.out | python ../bench2tex.py blue!50!black 128 > graph.tex
	./heap_set.out | python ../bench2tex.py blue!75!black 128 >> graph.tex
	./heap.out | python ../bench2tex.py blue 128 >> graph.tex
	./entt.out | python ../bench2tex.py green!75!black 128 >> graph.tex
	./tov.out | python ../bench2tex.py orange 128 >> graph.tex
	#./tov_tf.out | python ../bench2tex.py orange 128 >> graph.tex

bench_spawn.pdf: spawn.tex graph.tex
	pdflatex --jobname=bench_spawn spawn.tex

clean:
	rm -rf *.out *.d *.log *.aux bench_spawn.pdf

cleantex: clean
	rm -rf graph.tex

.PHONY: default run clean
