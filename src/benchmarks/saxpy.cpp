#include <cstring>

#include "util/benchmark.hpp"

struct X {
  float values[WIDTH];
};

struct Y {
  float values[WIDTH];
};

class SaxpySystem {
public:
  SaxpySystem(float a) : a(a) {}

  void operator()(const X& x, Y& y) const {
    for (auto i{0u}; i < WIDTH; ++i)
      y.values[i] = a * x.values[i] + y.values[i];
  }

private:
  float a;
};

class SpawnSystem {
public:
  auto operator()() const {
    return [](const auto& manager) {
      for (auto i{0u}; i < SPAWN_RATE; ++i)
        manager.new_entity(X{3.1416f}, Y{1.68f});
    };
  }
};
  
int main(int argc, const char** argv) {
  benchmark::Scene scene(
    SaxpySystem(9.81f),
    SpawnSystem{}
  );

  for (auto i{0u}; i < INITIAL_COUNT; ++i)
    scene->manager.new_entity(X{3.1416f}, Y{1.68f});

  scene.run();
  return 0;
}
