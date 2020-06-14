#pragma once

#include <iostream>
#include <tuple>
#include <vector>

#include "../include/bitset2/bitset2.hpp"

#include <boost/hana.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

namespace ecs::storage {

template<typename... TComponents>
class TupleOfVectors {
private:
  static constexpr auto _component_types = hana::tuple_t<TComponents...>;
  using Signature = Bitset2::bitset2<sizeof...(TComponents)>;

  template<typename TComponent>
  static constexpr size_t _component_index = hana::find_if(
    hana::make_range(0_c, hana::size_c<sizeof...(TComponents)>),
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
    _entities.resize(2);
    for (EntityMetadata& entity_metadata : _entities) entity_metadata.signature = signature_of<TComponents...>;
    [](auto...){}((std::get<std::vector<TComponents>>(_components).resize(2), 0)...);
    size = 2;
  }

  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    return std::get<std::vector<TComponent>>(_components)[entity];
  }

  template<typename... TRequiredComponents>
  void for_entities_with(auto callable) {
    constexpr Signature signature = signature_of<TRequiredComponents...>;
    std::cout << signature << std::endl;
    for (size_t i{0}; i < size; ++i) {
      if ((_entities[i].signature & signature) == signature)
        callable(Entity{i});
    }
  }

private:
  size_t size;
  std::vector<EntityMetadata> _entities;
  std::tuple<std::vector<TComponents>...> _components;
};

}