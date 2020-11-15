default: all

all: saxpy/bench.pdf saxpy_smart/bench.pdf component_types/bench.pdf component_types_systems/bench.pdf component_types_small_systems/bench.pdf spawn/bench.pdf bodies/bench.pdf spawn_entt/bench.pdf

%/bench.pdf: %/Makefile
	make -C $*

%/Makefile: %.py
	python $*.py

clean:
	rm -rfv saxpy saxpy_smart component_types component_types_systems component_types_small_systems spawn bodies spawn_entt

.PHONY: default all clean
