#pragma once

#include <iostream>
#include <tuple>
#include <vector>

#include "bitset2/bitset2.hpp"

#include <boost/hana.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

namespace ecs::storage {

/// Stores components in multiple equally-sized vectors.
///
/// For each stored component type there is exactly one vector.
/// All vectors are always of equal size, and always at least as long as the number of entities stored.
///
/// @tparam TStoredComponents The component types to be stored.
template<typename... TStoredComponents>
class TupleOfVectors {
private:
  static constexpr auto _component_types = hana::tuple_t<TStoredComponents...>;
  using Signature = Bitset2::bitset2<sizeof...(TStoredComponents)>;

  template<typename TComponent>
  static constexpr auto find_component_index() {
    auto index = hana::find_if(
      hana::make_range(0_c, hana::size_c<sizeof...(TStoredComponents)>),
      [](auto index) { return _component_types[index] == hana::type_c<TComponent>; }
    );
    static_assert(index != hana::nothing, "Component type is not stored.");
    return index.value();
  }

  template<typename TComponent>
  static constexpr size_t _component_index = find_component_index<TComponent>();

  template<typename... TRequiredComponents>
  static constexpr Signature signature_of = (Signature(0) |= ... |= (Signature(1) << _component_index<TRequiredComponents>));
public:
  /// The handle type for systems to reference entities with.
  using Entity = size_t;

  // TODO: REMOVE
  /// Constructs a storage with no components initially stored.
  TupleOfVectors() {
    static constexpr auto count = 0;
    _entities.resize(count);
    (std::get<std::vector<TStoredComponents>>(_components).resize(count), ...);
    // TODO: remove
    // for (Entity entity{0}; entity < count; ++entity)
    //   set_components(entity, TStoredComponents{}...);
    _capacity = count;
  }

  /// Returns the number of entities currently stored.
  ///
  /// @returns The number of entities currently stored.
  size_t get_size() {
    return _capacity;
  }

  /// Returns a reference to a single component of some entity.
  ///
  /// @param entity The entity to be accessed.
  /// @tparam TComponent The component type to be queried.
  /// @throws std::runtime_error when the queried component is not active on the entity.
  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    // TODO: static_assert component type handled
    return std::get<std::vector<TComponent>>(_components)[entity];
  }

  template<typename... TComponents>
  void set_components(Entity entity, TComponents&&... components) {
    // TODO: static_assert component type handled
    // Set associated component bits.
    _entities[entity].signature |= signature_of<std::decay_t<TComponents>...>;
    // Assign components from parameters.
    ((get_component<std::decay_t<TComponents>>(entity) = std::forward<TComponents>(components)), ...);
  }

  // TODO: return entity?
  void new_entity(auto&&... components) {
    ++_capacity;
    _entities.resize(_capacity);
    ((std::get<std::vector<TStoredComponents>>(_components).resize(_capacity), 0), ...);
    set_components(_capacity - 1, std::forward<decltype(components)>(components)...);
  }

  void remove_entity(Entity entity) {
    // TODO: reclaim
    _entities[entity].alive = false;
  }

  /// ...
  /// @requires No inactive entity to exist with an index smaller than the highest active one.
  template<typename... TRequiredComponents>
  void for_entities_with(auto callable) {
    if constexpr(sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      constexpr Signature signature = signature_of<TRequiredComponents...>;
      for (size_t i{0}; i < _capacity; ++i) {
        if ((_entities[i].signature & signature) == signature)
        // TODO: call with manager (since this is sequential)
          callable(Entity{i});
      }
    } else callable(Entity{0}); // TODO: move check to runtime to avoid 0-reservation
  }

  // TODO: parametrize parallelization
  template<typename... TRequiredComponents>
  void for_entities_with_parallel(auto callable) {
    if constexpr(sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      constexpr Signature signature = signature_of<TRequiredComponents...>;
      #pragma omp parallel for
      for (size_t i = 0; i < _capacity; ++i) {
        if ((_entities[i].signature & signature) == signature)
        // TODO: maybe a parallel manager?
          callable(Entity{i});
      }
    } else callable(Entity{0}); // TODO: move check to runtime
  }

  void print(std::ostream& stream) const {
    for (auto i{0u}; i < _capacity; ++i) {
      auto& entity{_entities[i]};
      stream << (entity.alive ? "E" : "_");
    }
    stream << std::endl;
  }

  /// ...
  /// After refreshing, no inactive entity lies before an active one in the vectors.
  auto refresh() {
    Entity it_inactive{0};
    Entity it_active{_capacity - 1};

    while (true) {
        while (true) {
            if (it_inactive > it_active) return it_inactive;
            if (!_entities[it_inactive].alive) break;
            it_inactive++;
        }
        while(true) {
            if (_entities[it_active].alive) break;
            // TODO: Update handles.
            // invalidateEntityHandle(it_active);
            if (it_active <= it_inactive) return it_inactive;
            it_active--;
        }
        assert(_entities[it_active].alive);
        assert(!_entities[it_inactive].alive);

        std::swap(_entities[it_active], _entities[it_inactive]);
        (([&]() {
          auto& component_vector = std::get<std::vector<TStoredComponents>>(_components);
          std::swap(component_vector[it_active], component_vector[it_inactive]);
        }(), 0), ...);

        // TODO: Update handles.
        // refreshEntityHandle(it_inactive);
        // invalidateEntityHandle(it_active);
        // refreshEntityHandle(it_active);

        it_inactive++;
        it_active--;
    }
    return it_inactive;
}


private:
  struct EntityMetadata {
    Signature signature;
    bool alive = true;
  };

  /// The number of entitites that memory is allocated for in the vectors.
  /// This includes inactive entitites.
  size_t _capacity = 0;
  /// The number of stored entitites that are active.
  size_t _size = 0;

  std::vector<EntityMetadata> _entities;
  std::tuple<std::vector<TStoredComponents>...> _components;
};

}