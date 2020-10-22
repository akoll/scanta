#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <cassert>

#include <bitset2/bitset2.hpp>

#include <boost/hana.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

#include "ecs/util/type_index.hpp"

namespace ecs::storage {

/// Stores components in multiple equally-sized vectors.
///
/// For each stored component type there is exactly one vector.
/// All vectors are always of equal size, and always at least as long as the number of entities stored.
///
/// @tparam TStoredComponents The component types to be stored.
template<typename... TStoredComponents>
class VectorOfTuples {
private:
  /// The list of stored component types as a hana::tuple_t.
  ///
  /// This allows handling the type list as a value instead of a template parameter pack,
  /// making it iterable and mutable with boost::hana functions.
  static constexpr auto _component_types = hana::tuple_t<TStoredComponents...>;

  /// The entity signature type.
  ///
  /// A bitset with a single bit for each component type.
  using Signature = Bitset2::bitset2<sizeof...(TStoredComponents)>;

  /// Field for accessing the index of a component type within the list of stored component types.
  ///
  /// @tparam TComponent The component type to access the index of.
  template<typename TComponent>
  static constexpr size_t _component_index = type_index<TComponent, TStoredComponents...>;

  /// An entity signature generated from a set of component types.
  ///
  /// The bit of each component type passed in is set, while all other bits stay off.
  /// @tparam TComponents The component types to be represented in the signature.
  template<typename... TComponents>
  // Use a fold expression (https://en.cppreference.com/w/cpp/language/fold) to construct the signature.
  // The bitset accumulator starts out at all zero. For each component type a corresponding signature
  // with just that bit set is created by shifting and included in the accumulator using a bitwise OR.
  static constexpr Signature signature_of = (Signature(0) |= ... |= (Signature(1) << _component_index<TComponents>));
public:
  /// The handle type for systems to reference entities with.
  using Entity = size_t;

  /// Returns the number of active entities currently stored.
  ///
  /// @returns The number of active entities currently stored.
  size_t get_size() {
    return _data.size();
  }

  /// Returns a reference to a single component of some entity.
  ///
  /// @param entity The entity to be accessed.
  /// @tparam TComponent The component type to be queried.
  /// @throws std::runtime_error when the queried component is not attached to the entity.
  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    // TODO: static_assert component type handled
    return std::get<TComponent>(_data[entity]);
  }

  /// Sets the component data for a single component of some entity.
  ///
  /// This also attaches the passed in component to this entity (i.e. the signature bit is set).
  /// All other components attached to this entity remain attached and unchanged.
  /// @param entity The entity for which to set the component.
  /// @param component The component data to be assigned.
  template<typename TComponent>
  void set_component(Entity entity, TComponent&& component) {
    // TODO: static_assert component type stored
    // Set the associated component bit in the entity signature.
    std::get<EntityMetadata>(_data[entity]).signature |= signature_of<std::decay_t<TComponent>>;
    // Assign component from the parameter.
    get_component<std::decay_t<TComponent>>(entity) = std::forward<TComponent>(component);
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
    // Set the associated component bits in the entity signature.
    std::get<EntityMetadata>(_data[entity]).signature = signature_of<std::decay_t<TComponents>...>;
    // Set all passed in components using a fold expression.
    (set_component(entity, std::forward<TComponents>(components)), ...);
  }

  /// Removes a component association from some entity.
  ///
  /// This disables the component on the entity by mutating the entity signature stored in the metadata.
  /// The component data is not cleared and its memory not released.
  template<typename TComponent>
  void remove_component(Entity entity) {
    // TODO: static_assert component type stored
    // Unset the associated component bit in the entity signature.
    std::get<EntityMetadata>(_data[entity]).signature &= !signature_of<std::decay_t<TComponent>>;
  }

  // TODO: return entity?
  /// Creates and activates a new entity.
  ///
  /// If necessary, this will allocate memory.
  /// Requires the entity slot at index `_size` to be inactive.
  /// @param components The set of components to be initially associated with the new entity.
  void new_entity(auto&&... components) {
    // Default-construct a new entity tuple.
    _data.emplace_back();
    // Set the initial components from the parameters.
    set_components(_data.size() - 1, std::forward<decltype(components)>(components)...);
  }

  /// Removes an entity from the storage.
  ///
  /// Sets the entity as inactive.
  /// Ensures that the activeness state of the entity at index `_size` is unchanged.
  void remove_entity(Entity entity) {
    _data.erase(_data.begin() + entity);
  }

  /// Execute a callable on each entity with all required components attached.
  ///
  /// Requires no inactive entity to exist with an index smaller than the highest active one.
  /// @tparam TRequiredComponents The set of component types required to be attached to an entity to be processed.
  /// @param callable The callable to be executed with each matched entity's index as an argument.
  template<typename... TRequiredComponents>
  void for_entities_with(auto&& callable) const {
    /// If the list of required component types is empty, the callable is called exactly once.
    if constexpr (sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      // Construct a signature to be matched against from the required component types.
      constexpr Signature signature = signature_of<TRequiredComponents...>;
      // Iterate all active components.
      for (size_t i{0}; i < _data.size(); ++i) {
        // Match the entity signature with the required component types using a bitwise AND.
        if ((std::get<EntityMetadata>(_data[i]).signature & signature) == signature)
        // TODO: call with manager (since this is sequential)
          callable(Entity{i});
      }
    } else callable(Entity{SIZE_MAX}); // TODO: move check to runtime to avoid -1-reservation (and also execute if ECS::Entity is required)
  }

  // TODO: document
  // TODO: parametrize parallelization
  template<typename... TRequiredComponents>
  void for_entities_with_parallel(auto&& callable) const {
    if constexpr (sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      static constexpr Signature signature = signature_of<TRequiredComponents...>;
      #pragma omp parallel for
      for (size_t i = 0; i < _data.size(); ++i) {
        if ((std::get<EntityMetadata>(_data[i]).signature & signature) == signature)
        // TODO: maybe a parallel manager?
          callable(Entity{i});
      }
    } else callable(Entity{SIZE_MAX}); // TODO: move check to runtime
  }

private:
  /// Contains any metadata (i.e. data besides the associated component data) necessary to be stored for an entity in this storage.
  struct EntityMetadata {
    /// The entity's signature.
    ///
    /// The state of a bit indicates whether the component of the corresponding component type is attached to the entity.
    /// A component is said to be attached to this entity if the bit is set, and detached if not.
    /// This is necessary because for each entity, storage for each component type is allocated and initialized (in the vectors)
    /// and its signature tracks if said memory is to be considered associated with the entity.
    Signature signature;
  };

  /// Tuple storing the entity metadata.
  ///
  /// The tuples storing component data, arranged in a vector.
  ///
  /// For each stored entity, a tuple is created which stores instances
  /// of all possible components.
  std::vector<std::tuple<EntityMetadata, TStoredComponents...>> _data;
};

}