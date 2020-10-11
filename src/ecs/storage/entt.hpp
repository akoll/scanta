#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>
#include <limits>

#include <boost/hana.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

#include <entt/entt.hpp>

#include "ecs/util/type_index.hpp"

namespace ecs::storage {

/// TODO: Stores components ...
///
/// @tparam TStoredComponents The component types to be stored.
template<typename... TStoredComponents>
class Entt {
private:
  /// The list of stored component types as a hana::tuple_t.
  ///
  /// This allows handling the type list as a value instead of a template parameter pack,
  /// making it iterable and mutable with boost::hana functions.
  static constexpr auto _component_types = hana::tuple_t<TStoredComponents...>;
public:
  /// The handle type for systems to reference entities with.
  ///
  /// This does _not_ use `entt::entity`, because the default entity type
  /// is a `uint32_t`, which imposes an entity count limit surpassed in this
  /// benchmark. Thus, a 64-bit integer is used instead.
  using Entity = std::uint64_t;

  /// Constructs a storage with no components initially stored.
  ///
  /// @param capacity The initial entity capacity for which to allocate memory for.
  Entt() {}

  /// Returns the number of active entities currently stored.
  size_t get_size() {
    return _registry.alive();
  }

  /// Returns a reference to a single component of some entity.
  ///
  /// @param entity The entity to be accessed.
  /// @tparam TComponent The component type to be queried.
  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    // TODO: static_assert component type handled
    return _registry.get<TComponent>(entity);
  }

  /// Sets the component data for a single component of some entity.
  ///
  /// All other components attached to this entity remain attached and unchanged.
  /// @param entity The entity for which to set the component.
  /// @param component The component data to be assigned.
  template<typename TComponent>
  void set_component(Entity entity, TComponent&& component) {
    // TODO: static_assert component type stored
    // _registry.get<TComponent>(entity) = std::forward<TComponent>(component);
    _registry.emplace_or_replace<TComponent>(entity, std::forward<TComponent>(component));
  }

  /// Sets the component data for some entity.
  ///
  /// Any components previously attached to the entity and not passed in again are detached.
  /// The passed in components make up the new complete set of components attached to that entity.
  /// @param entity The entity for which to set the components.
  /// @param components The components to be assigned.
  template<typename... TComponents>
  void set_components(Entity entity, TComponents&&... components) {
    // TODO: static_assert component type stored
    // Detach all components from the entity.
    _registry.remove_all(entity);
    // Set all passed in components using a fold expression.
    (set_component(entity, std::forward<TComponents>(components)), ...);
  }

  // TODO: Test if this function works.
  /// Removes a component association from some entity.
  ///
  /// This disables the component on the entity by mutating the entity signature stored in the metadata.
  /// The component data is not cleared and its memory not released.
  template<typename TComponent>
  void remove_component(Entity entity) {
    // TODO: static_assert component type stored
    _registry.remove<TComponent>(entity);
  }

  // TODO: return entity?
  /// Creates a new entity.
  ///
  /// @param components The set of components to be initially associated with the new entity.
  void new_entity(auto&&... components) {
    // Set the initial components from the parameters.
    Entity entity = _registry.create();
    set_components(entity, std::forward<decltype(components)>(components)...);
  }

  /// Removes an entity from the storage.
  void remove_entity(Entity entity) {
    _registry.destroy(entity);
  }

  /// Execute a callable on each entity with all required components attached.
  ///
  /// Requires no inactive entity to exist with an index smaller than the highest active one.
  /// @tparam TRequiredComponents The set of component types required to be attached to an entity to be processed.
  /// @param callable The callable to be executed with each matched entity's index as an argument.
  template<typename... TRequiredComponents>
  void for_entities_with(auto&& callable) {
    /// If the list of required component types is empty, the callable is called exactly once.
    if constexpr (sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      const auto view = _registry.view<TRequiredComponents...>();
      for (auto entity: view) callable(entity);
    } else callable(Entity{std::numeric_limits<std::uint64_t>::max()}); // TODO: move check to runtime to avoid -1-reservation (and also execute if ECS::Entity is required)
  }

  // TODO: document
  // TODO: parametrize parallelization
  template<typename... TRequiredComponents>
  void for_entities_with_parallel(auto&& callable) {
    /// If the list of required component types is empty, the callable is called exactly once.
    if constexpr (sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      auto view = _registry.view<TRequiredComponents...>();
      #pragma omp parallel for
      for (auto entity: view) callable(entity);
    } else callable(Entity{std::numeric_limits<unsigned int>::max()}); // TODO: move check to runtime to avoid -1-reservation (and also execute if ECS::Entity is required)
  }


private:
  /// The wrapped entt registry.
  ///
  /// This does _not_ use the default `eentt::registry`, because the default entity type
  /// (`entt::entity`) is a uint32_t, which imposes an entity count limit surpassed in this
  /// benchmark. Thus, a 64-bit integer is used instead.
  entt::basic_registry<std::uint64_t> _registry;
};

}
