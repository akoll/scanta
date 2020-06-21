#pragma once

#include <iostream>
#include <tuple>
#include <vector>

#include "bitset2/bitset2.hpp"

#include <boost/hana.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

namespace ecs::storage {

template<typename... TStoredComponents>
class TupleOfVectors {
private:
  static constexpr auto _component_types = hana::tuple_t<TStoredComponents...>;
  using Signature = Bitset2::bitset2<sizeof...(TStoredComponents)>;

  template<typename TComponent>
  static constexpr size_t _component_index = hana::find_if(
    hana::make_range(0_c, hana::size_c<sizeof...(TStoredComponents)>),
    [](auto index) { return _component_types[index] == hana::type_c<TComponent>; }
  ).value();

  template<typename TFirst>
  static constexpr auto make_signature() {
    return Signature(1) << _component_index<TFirst>;
  }

  template<typename TFirst, typename TSecond, typename... TRest>
  static constexpr auto make_signature() {
    return make_signature<TSecond, TRest...>() | (Signature(1) << _component_index<TFirst>);
  }

  template<typename... TRequiredComponents>
  static constexpr Signature signature_of = make_signature<TRequiredComponents...>();
public:
  using Entity = size_t;
  struct EntityMetadata {
      Signature signature;
  };

  // TODO: REMOVE
  TupleOfVectors() {
    static constexpr auto count = 512;
    _entities.resize(count);
    ((std::get<std::vector<TStoredComponents>>(_components).resize(count), 0), ...);
    for (Entity entity{0}; entity < count; ++entity)
      set_components(entity, TStoredComponents{}...);
    _size = count;
  }

  // TODO: REMOVE
  void add_entity() {
    ++_size;
    _entities.resize(_size);
    ((std::get<std::vector<TStoredComponents>>(_components).resize(_size), 0), ...);
    set_components(_size - 1, TStoredComponents{}...);
  }

  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    // TODO: static_assert component type handled
    return std::get<std::vector<TComponent>>(_components)[entity];
  }

  template<typename... TComponents>
  void set_components(Entity entity, TComponents&&... components) {
    // TODO: static_assert component type handled
    // Set associated component bits.
    _entities[entity].signature |= signature_of<TComponents...>;
    // Assign components from parameters.
    ((get_component<TComponents>(entity) = std::forward<TComponents>(components)), ...);
  }

  template<typename... TRequiredComponents>
  void for_entities_with(auto callable) {
    if constexpr(sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      constexpr Signature signature = signature_of<TRequiredComponents...>;
      for (size_t i{0}; i < _size; ++i) {
        if ((_entities[i].signature & signature) == signature)
          callable(Entity{i});
      }
    } else callable(Entity{0}); // TODO: reserve 0 as null-entity
  }

private:
  size_t _size = 0;
  std::vector<EntityMetadata> _entities;
  std::tuple<std::vector<TStoredComponents>...> _components;
};

}