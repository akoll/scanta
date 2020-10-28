#include <iostream>
#include <cstdlib>

#include "ecs/scaffold/ecs.hpp"

#include "../util/frame_limit_system.hpp"
#if defined BENCHMARK_FRAMETIME
#include "../util/frame_time_system.hpp"
#endif
#if defined BENCHMARK_MEMORY
#include "../util/memory_system.hpp"
#endif

#if defined RUNTIME_SEQUENTIAL
#include "ecs/runtime/sequential.hpp"
#elif defined RUNTIME_PARALLEL
#include "ecs/runtime/parallel.hpp"
#else
static_assert("No runtime strategy set.");
#endif

#if defined STORAGE_TOV
#include "ecs/storage/tuple_of_vectors.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::TupleOfVectors,
  #if defined RUNTIME_SEQUENTIAL
  ecs::runtime::Sequential
  #elif defined RUNTIME_PARALLEL
  ecs::runtime::Parallel
  #endif
>;
#elif defined STORAGE_VOT
#include "ecs/storage/vector_of_tuples.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::VectorOfTuples,
  #if defined RUNTIME_SEQUENTIAL
  ecs::runtime::Sequential
  #elif defined RUNTIME_PARALLEL
  ecs::runtime::Parallel
  #endif
>;
#elif defined STORAGE_SCATTERED
#include "ecs/storage/scattered.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::ScatteredCustom
    #ifdef STORAGE_SCATTERED_SMART
    ::WithSmartPointers
    #endif
    #ifdef STORAGE_SCATTERED_SET
      ::WithEntitySet
    #endif
    ::Storage
  ,
  #if defined RUNTIME_SEQUENTIAL
  ecs::runtime::Sequential
  #elif defined RUNTIME_PARALLEL
  ecs::runtime::Parallel
  #endif
>;
#elif defined STORAGE_ENTT
#include "ecs/storage/entt.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::Entt,
  #ifdef OUTER_PARALLEL
  ecs::runtime::Parallel
  #else
  ecs::runtime::Sequential
  #endif
>;
#else
static_assert(false, "No storage strategy set.");
#endif

#ifndef FRAME_COUNT
static_assert(false, "No frame count set.");
#endif

namespace benchmark {

template<typename... TSystems>
class Scene {
public:
  using BaseScene = ECS::Scene<
    TSystems...,
    #if defined BENCHMARK_FRAMETIME
    benchmark::FrameTimeSystem<FRAME_COUNT>,
    #endif
    #if defined BENCHMARK_MEMORY
    benchmark::MemoryUsageSystem<FRAME_COUNT>,
    #endif
    benchmark::FrameLimitSystem<FRAME_COUNT>
  >;

  Scene(TSystems&&... systems) :
    _scene(
      std::forward<TSystems>(systems)...,
      #if defined BENCHMARK_FRAMETIME
      benchmark::FrameTimeSystem<FRAME_COUNT>(),
      #endif
      #if defined BENCHMARK_MEMORY
      benchmark::MemoryUsageSystem<FRAME_COUNT>(),
      #endif
      benchmark::FrameLimitSystem<FRAME_COUNT>([this]() { _running = false; })
    )
  {}

  void operator()() {
    _running = true;
    while (_running) _scene();
  }

  operator BaseScene&() {
    return _scene;
  }

private:
  BaseScene _scene;
  bool _running;
};

template<typename... TSystems>
Scene(TSystems&&...) -> Scene<TSystems...>;

}
