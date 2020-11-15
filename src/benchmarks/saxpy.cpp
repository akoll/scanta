#include <cstring>

#include "util/benchmark.hpp"

struct X {
  float value[WIDTH];
};

struct Y {
  float value[WIDTH];
};

class SaxpySystem {
public:
  SaxpySystem(float a) : a(a) {}

  void operator()(const X& x, Y& y)
  #ifdef INNER_PARALLELISM
    const
  #endif
  {
    for (auto i{0u}; i < WIDTH; ++i)
      y.value[i] = a * x.value[i] + y.value[i];
  }

private:
  float a;
};

int main(int argc, const char** argv) {
  if (argc <= 1) return 1;
  unsigned int count = atoi(argv[1]);

  benchmark::Scene scene(SaxpySystem(9.81f));
  for (auto i{0u}; i < count; ++i)
    scene->manager.new_entity(X{3.1416f}, Y{1.68f});
  scene.run();
  return 0;
}
