#pragma once

namespace ecs {

// Manager interface for runtime operations.
// TODO: justify polymorphism
class RuntimeManager {
public:
  virtual size_t get_entity_count() = 0;
};

// Manager concept for deferred operations.
template<typename TDeferredManager>
concept DeferredManager = requires(TDeferredManager manager) {
   { manager.spawn_entity() };
};

}