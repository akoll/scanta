default: all

all: saxpy/bench.pdf saxpy_smart/bench.pdf component_types/bench.pdf spawn/bench.pdf spawn_mem/bench.pdf bodies/bench.pdf

%/bench.pdf: %/Makefile
	make -C $*

%/Makefile: %.py
	python $*.py

clean:
	rm -rfv saxpy saxpy_smart component_types spawn spawn_mem bodies

.PHONY: default all clean
