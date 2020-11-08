default: all

all: saxpy/bench.pdf component_types/bench.pdf spawn/bench.pdf bodies/bench.pdf

%/bench.pdf: %/Makefile
	make -C $*

%/Makefile: %.py
	python $*.py

clean:
	rm -rfv saxpy component_types spawn bodies

.PHONY: default all clean
