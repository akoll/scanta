#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <type_traits>


#include <boost/hana.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

namespace ecs::storage {

// Base declaration for partial specialization.
template<typename... TStoredComponents>
class Heap;

// Partial specialization for the case of no stored components.
// This is required because an entity handle type needs to be exposed
// to the scaffold before the list of stored components is known.
template<>
class Heap<> {
public:
  class Entity {
    template<typename...>
    friend class Heap;
  private:
    void* _pointer;
  public:
    Entity(void* pointer) : _pointer(pointer) {}
  };
};

template<typename... TStoredComponents>
class Heap : Heap<> {
private:
  static constexpr auto _component_types = hana::tuple_t<TStoredComponents...>;
public:
  struct EntityData {
    std::tuple<TStoredComponents*...> components;
  };
  // using Entity = EntityData*;

  class Entity {
  private:
    EntityData* _pointer;
  public:
    Entity(EntityData* pointer) : _pointer(pointer) {}

    Entity(Heap<>::Entity other) {
      _pointer = static_cast<EntityData*>(other._pointer);
    }

    operator Heap<>::Entity() {
      return static_cast<Heap<>::Entity>(_pointer);
    }

    operator EntityData*() {
      return _pointer;
    }

    EntityData* operator->() {
      return _pointer;
    }
  };

  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    // TODO: static_assert component type handled
    return *std::get<TComponent*>(entity->components);
  }

  size_t get_size() {
    return _entities.size();
  }

  template<typename... TComponents>
  void set_components(Entity entity, TComponents&&... components) {
    // TODO: static_assert component type handled
    // Copy components from parameters.
    ((std::get<std::decay_t<TComponents>*>(entity->components) = new TComponents(std::forward<TComponents>(components))), ...);
  }

  // TODO: return entity?
  void new_entity(auto&&... components) {
    EntityData* entity_data = new EntityData();
    _entities.push_back(entity_data);
    set_components(entity_data, std::forward<decltype(components)>(components)...);
  }

  void remove_entity(Entity entity) {
    auto it = std::find(_entities.begin(), _entities.end(), static_cast<EntityData*>(entity));
    if (it != _entities.end()) {
      delete *it;
      _entities.erase(it);
    }
    // else: entity not found
  }

  template<typename... TRequiredComponents>
  void for_entities_with(auto callable) {
    if constexpr(sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      for (EntityData* entity_data : _entities) {
        if ((... && std::get<TRequiredComponents*>(entity_data->components)))
          callable(entity_data);
      }
    } else callable(nullptr); // TODO: move check to runtime to avoid 0-reservation
  }

  // TODO: for_entities_with_parallel

private:
  std::vector<EntityData*> _entities;
};

}