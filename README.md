# Entity-Component-System
> Configurable header-only Entity-Component-System library.

## Dependencies
For compiling library headers:
* [gcc](https://gcc.gnu.org/) (tested on 10.2.0-2) with [OpenMP](https://www.openmp.org/) support
* [clang](https://clang.llvm.org/) is [not supported](http://clang.llvm.org/cxx_status.html) as of now, because it does not support [custom class instances as non-type template parameters](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0732r2.pdf)
* [boost libraries](https://www.boost.org/) ([hana](http://boostorg.github.io/hana/), [callable_traits](https://www.boost.org/doc/libs/1_67_0/libs/callable_traits/doc/html/index.html)) (tested on 1.72.0-2)

For running benchmarks:
* [python](https://www.python.org/) (tested on 3.8.5)
* [GNU make](https://www.gnu.org/software/make/)
* [GNU time](https://www.gnu.org/software/time/) (tested on 1.9-3)
* [perf](https://perf.wiki.kernel.org/index.php/Main_Page)
* [SDL2](https://www.libsdl.org/) (tested on 2.0.12-2)
* [LaTeX](https://www.latex-project.org/) (tested on texlive 2020.54586-5)

