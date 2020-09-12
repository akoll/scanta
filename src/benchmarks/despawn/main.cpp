#include <iostream>

#include "ecs/scaffold/ecs.hpp"

#include "ecs/storage/tuple_of_vectors.hpp"
#include "ecs/storage/heap.hpp"

#include "ecs/runtime/sequential.hpp"

#include "../util/frametime_system.hpp"

#if defined TUPLE_OF_VECTORS
using ECS = ecs::EntityComponentSystem<ecs::storage::TupleOfVectors, ecs::runtime::Sequential>;
#elif defined HEAP
using ECS = ecs::EntityComponentSystem<ecs::storage::ExplicitHeap, ecs::runtime::Sequential>;
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

class DespawnCountSystem {
public:
  size_t count = 0;
  auto operator()() {
    count = 0;
  }
};

class DespawnSystem {
public:
  auto operator()(ECS::Entity entity, DespawnCountSystem& despawn_count_system, BigBoi&) {
    auto count = despawn_count_system.count++;
    return [count, entity](const auto& manager) {
      if (count < 64) manager.remove_entity(entity);
    };
  }
};

int main() {
  bool running;

  ECS::Runtime scene(
    BigSystem{},
    DespawnCountSystem{},
    DespawnSystem{},
    benchmark::FrametimeSystem<5000>([&running]() { running = false; })
  );

  for (auto i{0u}; i < 320000; ++i) scene.manager.new_entity(BigBoi{});

  running = true;
  while (running) {
    scene();
  }

  return 0;
}
