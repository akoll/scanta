#include <cstring>

#include "util/benchmark.hpp"

struct Payload {
  size_t value[256];
};

class PayloadSystem {
public:
  void operator()(const Payload&) const {}
};

class SpawnSystem {
public:
  auto operator()() {
    return [](const auto& manager) {
      for (auto i{0u}; i < SPAWN_RATE; ++i)
        manager.new_entity(Payload{});
    };
  }
};
  
int main(int argc, const char** argv) {
  benchmark::Scene scene(
    PayloadSystem{},
    SpawnSystem{}
  );
  for (auto i{0u}; i < INITIAL_COUNT; ++i)
    scene->manager.new_entity(Payload{});
  scene.run();
  return 0;
}
