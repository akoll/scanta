#include <iostream>

#include "ecs/scaffold/ecs.hpp"

#include "ecs/storage/tuple_of_vectors.hpp"
#include "ecs/storage/heap.hpp"

#include "ecs/runtime/sequential.hpp"
#include "ecs/runtime/parallel.hpp"

#include "../util/frametime_system.hpp"

#if defined TUPLE_OF_VECTORS
using ECS = ecs::EntityComponentSystem<
  ecs::storage::TupleOfVectors,
  #ifdef OUTER_PARALLEL
  ecs::runtime::Parallel
  #else
  ecs::runtime::Sequential
  #endif
>;
#elif defined HEAP
using ECS = ecs::EntityComponentSystem<
  ecs::storage::CustomHeap
    #ifdef HEAP_SMART
    ::WithSmartPointers
    #endif
    #ifdef HEAP_SET
      ::WithEntitySet
    #endif
    ::Storage
  ,
  #ifdef OUTER_PARALLEL
  ecs::runtime::Parallel
  #else
  ecs::runtime::Sequential
  #endif
>;
#else
static_assert(false, "No storage strategy set.");
#endif

volatile size_t screen;

struct BiggestBoi {
  std::array<size_t, 11> stuff;
};

struct BigBoi {
  std::array<size_t, 10> stuff;
};

struct SmallBoi {
  std::array<size_t, 9> stuff;
};

struct SmallerBoi {
  std::array<size_t, 8> stuff;
};

constexpr auto iters =16u;

class BiggestSystem {
public:
  void operator()(BiggestBoi& big_boi) {
    size_t sum = 0;
    for (auto kek{0u}; kek < iters; ++kek)
      for (auto i{0u}; i < big_boi.stuff.size(); ++i) sum += ++big_boi.stuff[i] * i;
    screen = sum;
  }
};

class BigSystem {
public:
  void operator()(BigBoi& big_boi) {
    size_t sum = 0;
    for (auto kek{0u}; kek < iters; ++kek)
      for (auto i{0u}; i < big_boi.stuff.size(); ++i) sum += ++big_boi.stuff[i] * i;
    screen = sum;
  }
};

class SmallSystem {
public:
  void operator()(SmallBoi& small_boi) {
    size_t sum = 0;
    for (auto kek{0u}; kek < iters; ++kek)
      for (auto i{0u}; i < small_boi.stuff.size(); ++i) sum += ++small_boi.stuff[i] * i;
    screen = sum;
  }
};

class SmallerSystem {
public:
  void operator()(SmallerBoi& small_boi) {
    size_t sum = 0;
    for (auto kek{0u}; kek < iters; ++kek)
      for (auto i{0u}; i < small_boi.stuff.size(); ++i) sum += ++small_boi.stuff[i] * i;
    screen = sum;
  }
};

class SpawnSystem {
public:
  auto operator()() {
    return [](const auto& manager) {
      for (auto i{0u}; i < 50000; ++i)
        manager.new_entity(BiggestBoi{}, BigBoi{}, SmallBoi{}, SmallerBoi{});
    };
  }
};

int main() {
  bool running;

  ECS::Runtime scene(
    BiggestSystem{},
    BigSystem{},
    SmallSystem{},
    SmallerSystem{},
    SpawnSystem{},
    benchmark::FrametimeSystem<100>([&running]() { running = false; })
  );

  for (auto i{0u}; i < 50000; ++i)
	  scene.manager.new_entity(BiggestBoi{}, BigBoi{}, SmallBoi{}, SmallerBoi{});

  running = true;
  while (running) {
    scene();
  }

  return 0;
}
