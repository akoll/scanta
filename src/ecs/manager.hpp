#pragma once

#include <memory>

namespace ecs {

// Manager interface for runtime operations.
// TODO: justify polymorphism
class RuntimeManager {
public:
  virtual size_t get_entity_count() = 0;
};


// Manager interface for deferred operations.
// TODO: justify polymorphism
class DeferredManager : public virtual RuntimeManager {
public:
  virtual void spawn_entity() = 0;
};

}