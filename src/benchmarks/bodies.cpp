#include <cstring>

#include "util/benchmark.hpp"

/// This benchmark emulates the behavior of the _tuple of vectors of tuples_ storage.
/// This is achieved by using a _tuple of vectors_ storage but manually storing tuples inside.

struct Transform {
  float position[WIDTH];
};

struct RigidBody {
  float mass[WIDTH];
};

struct SoftBody {
  float stiffness[WIDTH];
};

class RigidBodySystem {
public:
  #if defined OPTIMIZE_RIGID
  void operator()(std::tuple<Transform, RigidBody>& tuple) const {
    #ifndef SKIP_RIGID
    for (auto i{0}; i < WIDTH; ++i)
      std::get<Transform>(tuple).position[i] += std::get<RigidBody>(tuple).mass[i];
    #endif
  }
  #elif defined OPTIMIZE_SOFT
  void operator()(std::tuple<Transform, SoftBody>& tuple, const RigidBody& rigid_body) const {
    #ifndef SKIP_RIGID
    for (auto i{0}; i < WIDTH; ++i)
      std::get<Transform>(tuple).position[i] += rigid_body.mass[i];
    #endif
  }
  #else
  static_assert(false, "No system optimization chosen.");
  #endif
};

class SoftBodySystem {
public:
  #if defined OPTIMIZE_SOFT
  void operator()(std::tuple<Transform, SoftBody>& tuple) const {
    #ifndef SKIP_SOFT
    for (auto i{0}; i < WIDTH; ++i)
      std::get<Transform>(tuple).position[i] += std::get<SoftBody>(tuple).stiffness[i];
    #endif
  }
  #elif defined OPTIMIZE_RIGID
  void operator()(std::tuple<Transform, RigidBody>& tuple, const SoftBody& soft_body) const {
    #ifndef SKIP_SOFT
    for (auto i{0}; i < WIDTH; ++i)
      std::get<Transform>(tuple).position[i] += soft_body.stiffness[i];
    #endif
  }
  #else
  static_assert(false, "No system optimization chosen.");
  #endif
};

int main(int argc, const char** argv) {
  if (argc <= 1) return 1;
  unsigned int count = atoi(argv[1]);

  #if defined OPTIMIZE_RIGID
  benchmark::Scene scene(RigidBodySystem{}, SoftBodySystem{});
  for (auto i{0u}; i < count; ++i)
    scene->manager.new_entity(std::make_tuple(Transform{}, RigidBody{}), SoftBody{});
  #elif defined OPTIMIZE_SOFT
  benchmark::Scene scene(SoftBodySystem{}, RigidBodySystem{});
  for (auto i{0u}; i < count; ++i)
    scene->manager.new_entity(std::make_tuple(Transform{}, SoftBody{}), RigidBody{});
  #endif

  scene.run();
  return 0;
}
