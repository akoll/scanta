#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <memory>
#include <type_traits>


#include <boost/hana.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

namespace ecs::storage {

template<typename... TStoredComponents>
class SmartHeap;

template<>
class SmartHeap<> {
public:
  class Entity {
    template<typename...>
    friend class SmartHeap;
  private:
    std::shared_ptr<void> _pointer;
  public:
    Entity(std::shared_ptr<void> pointer) : _pointer(pointer) {}
  };
};

template<typename... TStoredComponents>
class SmartHeap : SmartHeap<> {
private:
  static constexpr auto _component_types = hana::tuple_t<TStoredComponents...>;
public:
  struct EntityData {
    std::tuple<std::shared_ptr<TStoredComponents>...> components;
  };
  // using Entity = EntityData*;

  class Entity {
  private:
    std::shared_ptr<EntityData> _pointer;
  public:
    Entity(std::shared_ptr<EntityData> pointer) : _pointer(pointer) {}

    Entity(SmartHeap<>::Entity other) {
      _pointer = std::static_pointer_cast<EntityData>(other._pointer);
    }

    operator SmartHeap<>::Entity() {
      return std::static_pointer_cast<SmartHeap<>::Entity>(_pointer);
    }

    operator std::shared_ptr<EntityData>() {
      return _pointer;
    }

    std::shared_ptr<EntityData> operator->() {
      return _pointer;
    }
  };

  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    // TODO: static_assert component type handled
    return *std::get<std::shared_ptr<TComponent>>(entity->components);
  }

  size_t get_size() {
    return _entities.size();
  }

  template<typename... TComponents>
  void set_components(Entity entity, TComponents&&... components) {
    // TODO: static_assert component type handled
    // Copy components from parameters.
    ((std::get<std::shared_ptr<std::decay_t<TComponents>>>(entity->components) = std::make_shared<TComponents>(std::forward<TComponents>(components))), ...);
  }

  // TODO: return entity?
  void new_entity(auto&&... components) {
    std::shared_ptr<EntityData> entity_data = std::make_shared<EntityData>();
    _entities.push_back(entity_data);
    set_components(entity_data, std::forward<decltype(components)>(components)...);
  }

  void remove_entity(Entity entity) {
    auto it = std::find(_entities.begin(), _entities.end(), static_cast<std::shared_ptr<EntityData>>(entity));
    if (it != _entities.end()) {
      _entities.erase(it);
    }
    // else: entity not found
  }

  template<typename... TRequiredComponents>
  void for_entities_with(auto callable) {
    if constexpr(sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      for (std::shared_ptr<EntityData> entity_data : _entities) {
        if ((... && std::get<std::shared_ptr<TRequiredComponents>>(entity_data->components)))
          callable(static_cast<SmartHeap<>::Entity>(entity_data));
      }
    } else callable(SmartHeap<>::Entity(nullptr)); // TODO: move check to runtime to avoid 0-reservation
  }

  // TODO: for_entities_with_parallel

private:
  std::vector<std::shared_ptr<EntityData>> _entities;
};

}