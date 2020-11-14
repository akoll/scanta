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

/// TODO: Stores component data in a vector of entity tuples.
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

  /// Constructs a storage with no components initially stored.
  ///
  /// @param capacity The initial entity capacity for which to allocate memory for.
  VectorOfTuples(size_t capacity = 32) {
    _data.reserve(capacity);
  }

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
  /// This merely sets the entity as inactive. Shuffling will later reclaim the storage space.
  void remove_entity(Entity entity) {
    // Set the entity as inactive.
    std::get<EntityMetadata>(_data[entity]).active = false;
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

  /// Refreshes the storage to restore the preconditions necessary for iterating the entities.
  auto refresh() {
    size_t size = shuffle();
    _data.resize(size);
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

    /// The entity's activeness. This represents the entities's existence/presence. Instead of being removed from memory,
    /// entities to be removed are simply set as inactive. Shuffling then later gets rid of them.
    bool active = true;
  };

  /// Tuple storing the entity metadata.
  ///
  /// The tuples storing component data, arranged in a vector.
  ///
  /// For each stored entity, a tuple is created which stores instances
  /// of all possible components.
  std::vector<std::tuple<EntityMetadata, TStoredComponents...>> _data;

  /// Rearrange entity metadata and component data to have all active entities packed sequentially.
  ///
  /// This is essentially quicksort on a list of binary values (the `active` booleans), which takes only one iteration (no recursion required).
  ///
  /// After shuffling, no inactive entity lies before an active one in the vectors.
  /// Thus all active entities can be iterated contiguously.
  /// @returns The index of the first inactive entity in the storage. This is also the
  /// number of active entities preceding it (and thus the total number of currently active stored entities).
  size_t shuffle() {
    // If the storage is empty, return immediately.
    if (_data.size() == 0) return 0;

    // There are two iterators, initially pointing to the beginning and the end of the vectors respectively.
    // They then move inwards toward each other and swap their elements whenever an inactive one and an active one
    // are found with the inactive one having a lower index.
    // The left iterator (`it_inactive`) only stops on inactive entities, while the right iterator (`it_active`) stops on active ones.

    // Start out at index 0 (first element / left).
    Entity it_inactive{0};
    // Start out at index `_entities.size() - 1` (last element / right).
    Entity it_active{_data.size() - 1};

    // Loop indefinetely. Return conditions are given by the iterators crossing over.
    while (true) {
      // Move the left iterator to the right until it hits an inactive entity.
      while (true) {
        // If the left iterator is further right than the right one, all entities have been processed.
        if (it_inactive > it_active) return it_inactive;
        // If the left iterator points to an inactive entity, stop moving.
        if (!std::get<EntityMetadata>(_data[it_inactive]).active) break;
        // Move the left iterator to the right.
        it_inactive++;
      }
      // Move the right iterator to the left until it hits an active entity.
      while (true) {
        // If the right iterator points to an active entity, stop moving.
        if (std::get<EntityMetadata>(_data[it_active]).active) break;
        // TODO: Update handles.
        // invalidateEntityHandle(it_active);
        // If the right iterator moves overtop the left one, all entities have been processed.
        if (it_active <= it_inactive) return it_inactive;
        // Move the right iterator to the left.
        it_active--;
      }

      // After moving, the right iterator points to an active entity,
      assert(std::get<EntityMetadata>(_data[it_active]).active);
      // After moving, the left iterator points to an inactive entity,
      assert(!std::get<EntityMetadata>(_data[it_inactive]).active);

      // Swap the active and the inactive entities, so that the active is left of the inactive.
      std::swap(_data[it_active], _data[it_inactive]);

      // TODO: Update handles.
      // refreshEntityHandle(it_inactive);
      // invalidateEntityHandle(it_active);
      // refreshEntityHandle(it_active);

      // Move iterators towards each other to continue.
      it_inactive++;
      it_active--;
    }
  }

};

}
