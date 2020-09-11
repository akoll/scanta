#include <iostream>

#include "ecs/scaffold/ecs.hpp"
#include "ecs/scaffold/manager.hpp"

#include "ecs/storage/tuple_of_vectors.hpp"
#include "ecs/storage/heap.hpp"
#include "ecs/storage/heap_smart.hpp"

#include "ecs/runtime/sequential.hpp"

#include "../util/frametime_system.hpp"

#if defined TUPLE_OF_VECTORS
using ECS = ecs::EntityComponentSystem<ecs::storage::TupleOfVectors, ecs::runtime::Sequential>;
#elif defined HEAP
using ECS = ecs::EntityComponentSystem<ecs::storage::Heap, ecs::runtime::Sequential>;
#elif defined HEAP_SMART
using ECS = ecs::EntityComponentSystem<ecs::storage::SmartHeap, ecs::runtime::Sequential>;
#endif

volatile size_t screen;

struct BigBoi {
  std::array<size_t, 5> stuff;
};

class BigSystem {
public:
  void operator()(BigBoi& big_boi) {
    size_t sum = 0;
    for (auto i{0u}; i < big_boi.stuff.size(); ++i) sum += ++big_boi.stuff[i] * i;
    screen = sum;
  }
};

class SpawnSystem {
public:
  auto operator()() {
    return [](const auto& manager) {
      for (auto i{0u}; i < 128; ++i)
        manager.new_entity(BigBoi{});
    };
  }
};

int main() {
  bool running;

  ECS::Runtime scene(
    BigSystem{},
    SpawnSystem{},
    benchmark::FrametimeSystem<5000>([&running]() { running = false; })
  );

  running = true;
  while (running) {
    scene();
  }

  return 0;
}
