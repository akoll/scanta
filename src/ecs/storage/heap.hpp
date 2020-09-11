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

// Base declaration for partial specialization.
template<bool smart, typename... TStoredComponents>
class Heap;

// Partial specialization for the case of no stored components.
// This is required because an entity handle type needs to be exposed
// to the scaffold before the list of stored components is known.
template<bool smart>
class Heap<smart> {
public:
  class Entity {
    template<bool, typename...>
    friend class Heap;
  private:
    using VoidPointer = typename std::conditional<smart, std::shared_ptr<void>, void*>::type;
    VoidPointer _pointer;
  public:
    Entity(VoidPointer pointer) : _pointer(pointer) {}
  };
};

template<bool smart, typename... TStoredComponents>
class Heap : Heap<smart> {
private:
  static constexpr auto _component_types = hana::tuple_t<TStoredComponents...>;

  template<typename T>
  using Pointer = typename std::conditional<smart, std::shared_ptr<T>, T*>::type;
public:
  struct EntityData {
    std::tuple<Pointer<TStoredComponents>...> components;
  };
  // using Entity = EntityData*;

  class Entity {
  private:
    Pointer<EntityData> _pointer;
  public:
    Entity(Pointer<EntityData> pointer) : _pointer(pointer) {}

    Entity(typename Heap<smart>::Entity other) {
      if constexpr (smart)
        _pointer = std::static_pointer_cast<EntityData>(other._pointer);
      else
        _pointer = static_cast<EntityData*>(other._pointer);
    }

    operator typename Heap<smart>::Entity() {
      if constexpr (smart)
        return std::static_pointer_cast<Heap<smart>::Entity>(_pointer);
      else
        return static_cast<typename Heap<smart>::Entity>(_pointer);
    }

    operator Pointer<EntityData>() {
      return _pointer;
    }

    Pointer<EntityData> operator->() {
      return _pointer;
    }
  };

  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    // TODO: static_assert component type handled
    return *std::get<Pointer<TComponent>>(entity->components);
  }

  size_t get_size() {
    return _entities.size();
  }

  template<typename... TComponents>
  void set_components(Entity entity, TComponents&&... components) {
    // TODO: static_assert component type handled
    // Copy components from parameters.
    if constexpr (smart)
      ((std::get<std::shared_ptr<std::decay_t<TComponents>>>(entity->components) = std::make_shared<TComponents>(std::forward<TComponents>(components))), ...);
    else
      ((std::get<std::decay_t<TComponents>*>(entity->components) = new TComponents(std::forward<TComponents>(components))), ...);
  }

  // TODO: return entity?
  void new_entity(auto&&... components) {
    Pointer<EntityData> entity_data;
    if constexpr (smart)
      entity_data = std::make_shared<EntityData>();
    else
      entity_data = new EntityData();
    _entities.push_back(entity_data);
    set_components(entity_data, std::forward<decltype(components)>(components)...);
  }

  void remove_entity(Entity entity) {
    auto it = std::find(_entities.begin(), _entities.end(), static_cast<Pointer<EntityData>>(entity));
    if (it != _entities.end()) {
      if constexpr (!smart) delete *it;
      _entities.erase(it);
    }
    // else: entity not found
  }

  template<typename... TRequiredComponents>
  void for_entities_with(auto callable) {
    if constexpr(sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      for (Pointer<EntityData> entity_data : _entities) {
        if ((... && std::get<Pointer<TRequiredComponents>>(entity_data->components)))
          callable(static_cast<typename Heap<smart>::Entity>(entity_data));
      }
    } else callable(typename Heap<smart>::Entity(nullptr)); // TODO: move check to runtime to avoid 0-reservation
  }

  // TODO: for_entities_with_parallel

private:
  std::vector<Pointer<EntityData>> _entities;
};

template<typename... TComponents>
class ExplicitHeap : public Heap<false, TComponents...> {};

template<typename... TComponents>
class SmartHeap : public Heap<true, TComponents...> {};

}
