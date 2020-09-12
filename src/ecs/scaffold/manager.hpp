#pragma once

namespace ecs {

// Manager interface for runtime operations.
// TODO: justify polymorphism
class RuntimeManager {
public:
  virtual size_t get_entity_count() const = 0;
};

}
