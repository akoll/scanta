#include <iostream>
#include <cstdlib>

#include "ecs/scaffold/ecs.hpp"

#include "ecs/runtime/sequential.hpp"
#if defined OUTER_PARALLEL
#include "ecs/runtime/parallel.hpp"
#endif

#include "../util/frame_limit_system.hpp"
#include "../util/frame_time_system.hpp"
// TODO: only include when used
#include "../util/memory_system.hpp"

#if defined CONTIGUOUS
#include "ecs/storage/tuple_of_vectors.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::TupleOfVectors,
  #if not defined OUTER_PARALLEL
  ecs::runtime::Sequential
  #else
  ecs::runtime::Parallel
  #endif
>;
#elif defined VOT
#include "ecs/storage/vector_of_tuples.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::VectorOfTuples,
  #if not defined OUTER_PARALLEL
  ecs::runtime::Sequential
  #else
  ecs::runtime::Parallel
  #endif
>;
#elif defined SCATTERED
#include "ecs/storage/scattered.hpp"
using ECS = ecs::EntityComponentSystem<
  ecs::storage::ScatteredCustom
    #ifdef HEAP_SMART
    ::WithSmartPointers
    #endif
    #ifdef HEAP_SET
      ::WithEntitySet
    #endif
    ::Storage
  ,
  #if not defined OUTER_PARALLEL
  ecs::runtime::Sequential
  #else
  ecs::runtime::Parallel
  #endif
>;
#elif defined ENTT
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
  using Runtime = ECS::Runtime<
    TSystems...,
    benchmark::MemoryUsageSystem<FRAME_COUNT>,
    benchmark::FrameTimeSystem<FRAME_COUNT>,
    benchmark::FrameLimitSystem<FRAME_COUNT>
  >;

  Scene(TSystems&&... systems) :
    _scene(
      std::forward<TSystems>(systems)...,
      benchmark::MemoryUsageSystem<FRAME_COUNT>(),
      benchmark::FrameTimeSystem<FRAME_COUNT>(),
      benchmark::FrameLimitSystem<FRAME_COUNT>([this]() { _running = false; })
    )
  {}

  void operator()() {
    _running = true;
    while (_running) _scene();
  }

  operator Runtime&() {
    return _scene;
  }

private:
  Runtime _scene;
  bool _running;
};

template<typename... TSystems>
Scene(TSystems&&...) -> Scene<TSystems...>;

}
