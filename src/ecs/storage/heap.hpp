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

/// Base declaration for partial specialization.
template<bool smart, typename... TStoredComponents>
class Heap;

/// Partial specialization for the case of no stored components.
///
/// This is required because an entity handle type needs to be exposed
/// to the scaffold before the list of stored components is known.
/// @tparam smart Whether to use [smart pointers](https://en.cppreference.com/book/intro/smart_pointers) or regular pointers.
template<bool smart>
class Heap<smart> {
public:
  /// The base entity handle type of the heap storage.
  ///
  /// This entity handle type is used for the declaration of systems. A heap storage instance with stored component types specified
  /// defines another entity handle type, which can be converted from and to this one.
  /// A void-pointer is used as the handle type, since the underlying entity metadata type is not known without component types.
  class Entity {
    // All other Heap classes are friends, so they can access the void-pointer for casting.
    template<bool, typename...>
    friend class Heap;
  private:
    /// The void-pointer type for this storage.
    ///
    /// If smart pointers are used, uses a shared pointer, otherwise a regular pointer.
    using VoidPointer = typename std::conditional<smart, std::shared_ptr<void>, void*>::type;
    VoidPointer _pointer;
  public:
    /// Constructor for converting from plain void-pointers.
    Entity(VoidPointer pointer) : _pointer(pointer) {}
  };
};

/// Stores components and entity metadata in dynamically allocated and scattered heap regions.
///
/// @tparam smart Whether to use [smart pointers](https://en.cppreference.com/book/intro/smart_pointers) or regular pointers.
/// @tparam TStoredComponents The component types to be stored.
template<bool smart, typename... TStoredComponents>
class Heap : Heap<smart> {
private:
  /// The list of stored component types as a hana::tuple_t.
  ///
  /// This allows handling the type list as a value instead of a template parameter pack,
  /// making it iterable and mutable with boost::hana functions.
  static constexpr auto _component_types = hana::tuple_t<TStoredComponents...>;

  /// Template for generating pointer types.
  ///
  /// While the underlying pointer types are different depending on whether to use smart pointers,
  /// this type alias allows handling them equivalently.
  /// When using smart pointers, a shared pointer is used, otherwise a regular pointer is used.
  template<typename T>
  using Pointer = typename std::conditional<smart, std::shared_ptr<T>, T*>::type;
public:
  /// Entity metadata used by the storage internally.
  struct EntityData {
    /// The associated components are referenced in a tuple of pointers.
    /// For each component type, a pointer is stored, pointing to the component data.
    /// When a component is not enabled for the entity, the pointer is null.
    std::tuple<Pointer<TStoredComponents>...> components;

    /// Deletes all components associated with this entity.
    void clear_components() {
      // If regular pointers are used, memory needs to be freed first using `delete`.
      if constexpr (!smart)
        ((delete std::get<Pointer<std::decay_t<TStoredComponents>>>(components)), ...);
      // Set all pointer to null using a fold-expression.
      ((std::get<Pointer<std::decay_t<TStoredComponents>>>(components) = nullptr), ...);
    }

    /// Destructor which deletes all components.
    ~EntityData() {
      if constexpr (!smart)
        ((delete std::get<Pointer<std::decay_t<TStoredComponents>>>(components)), ...);
    }
  };

  /// The entity handle type of the heap storage.
  ///
  /// This entity handle type encapsulates the entity metadata with respect to the stored component types.
  /// In the system definitions the type used is the one from the heap storage without stored components.
  /// Both handle types are implicitly convertible, which makes this work.
  class Entity {
  private:
    /// The underlying metadata pointer.
    Pointer<EntityData> _pointer;
  public:
    /// Constructor for converting from plain pointers.
    Entity(Pointer<EntityData> pointer) : _pointer(pointer) {}

    /// Constructor for converting from the base entity handle type (used in system definitions).
    Entity(typename Heap<smart /* no stored component types */>::Entity other) {
      // Cast the void-pointer in the base handle to the appropriate handle from this storage.
      if constexpr (smart)
        _pointer = std::static_pointer_cast<EntityData>(other._pointer);
      else
        _pointer = static_cast<EntityData*>(other._pointer);
    }

    /// Conversion operator for converting to a base handle.
    operator typename Heap<smart>::Entity() {
      // Construct a base handle from the metadata handle.
      if constexpr (smart)
        return std::static_pointer_cast<Heap<smart>::Entity>(_pointer);
      else
        return static_cast<typename Heap<smart>::Entity>(_pointer);
    }

    /// Conversion operator for converting to a plain pointer.
    operator Pointer<EntityData>() {
      return _pointer;
    }

    /// Dereferencing pointer access operator.
    Pointer<EntityData> operator->() {
      return _pointer;
    }
  };


  /// Returns a reference to a single component of some entity.
  ///
  /// @param entity The entity to be accessed.
  /// @tparam TComponent The component type to be queried.
  /// @throws std::runtime_error when the queried component is not active on the entity.
  template<typename TComponent>
  TComponent& get_component(Entity entity) {
    // TODO: static_assert component type handled
    // Fetch the component pointer from the entity metadata tuple and dereference it.
    return *std::get<Pointer<TComponent>>(entity->components);
  }

  /// Returns the number of active entities currently stored.
  ///
  /// The number of active entities is the number of elements in the entity metadata vector.
  size_t get_size() {
    return _entities.size();
  }

  // TODO: Add set_component for single components?

  /// Sets the component data for some entity.
  ///
  /// Any components previously enabled on the entity and not passed in again are disabled.
  /// The passed in components make up the new complete set of components for that entity.
  /// Components are passed in as temporary rvalue-references. For each one, memory is then allocated
  /// and the component temporary is copied into the heap. The resulting pointer is stored in the entity metadata tuple.
  /// @param entity The entity for which to set the components.
  /// @params components The component data rvalues to be assigned.
  template<typename... TComponents>
  void set_components(Entity entity, TComponents&&... components) {
    // TODO: static_assert component type handled
    // Remove all components from the entity.
    entity->clear_components();
    // Copy components from parameters.
    if constexpr (smart)
      // The component rvalue parameter-pack is unpacked using a fold-expression.
      (
        // Use make_shared to create a shared smart pointer.
        (std::get<Pointer<std::decay_t<TComponents>>>(entity->components) = std::make_shared<TComponents>(std::forward<TComponents>(components))),
        ... // Repeat for every component passed in.
      );
    else
      // The component rvalue parameter-pack is unpacked using a fold-expression.
      (
        // Use `new` to directly allocate memory for the component.
        (std::get<Pointer<std::decay_t<TComponents>>>(entity->components) = new TComponents(std::forward<TComponents>(components))),
        ... // Repeat for every component passed in.
      );
  }

  // TODO: Add remove_component?

  // TODO: return entity?
  /// Create a new entity.
  ///
  /// Allocates new memory for the entity metadata.
  /// @param components The set of components to be initially associated with the new entity.
  void new_entity(auto&&... components) {
    // Pointer to the new entity metadata.
    Pointer<EntityData> entity_data;
    // Construct either shared or plain.
    if constexpr (smart)
      entity_data = std::make_shared<EntityData>();
    else
      entity_data = new EntityData();
    // Add the new metadata pointer to the entity vector.
    _entities.push_back(entity_data);
    // Set initial components by forwarding them (retaining references without copy).
    set_components(entity_data, std::forward<decltype(components)>(components)...);
  }

  /// Removes an entity from the storage.
  ///
  /// Frees any memory for the entity metadata and associated components.
  void remove_entity(Entity entity) {
    // Find the entity in the vector of pointers. This is O(N).
    auto it = std::find(_entities.begin(), _entities.end(), static_cast<Pointer<EntityData>>(entity));
    // If the entity has been found (/ is stored).
    if (it != _entities.end()) {
      // Delete it. This also deletes all components.
      if constexpr (!smart) delete *it;
      // Remove it from the entity vector.
      _entities.erase(it);
    }
    // TODO: else: entity not found
  }

  /// Execute a callable on each entity with all required components enabled.
  ///
  /// @tparam TRequiredComponents The set of component types required to be enabled for an entity to be processed.
  /// @param callable The callable to be executed with each matched entity's index as an argument.
  template<typename... TRequiredComponents>
  void for_entities_with(auto callable) {
    /// If the list of required component types is empty, the callable is called exactly once.
    if constexpr(sizeof...(TRequiredComponents) > 0) {
      // TODO: static_assert component types handled
      // Iterate all stored entities.
      for (Pointer<EntityData> entity_data : _entities) {
        // Check the entity signature by seeing if all required component pointers are non-null.
        // This is done using a fold-expression with the boolean AND operator. Since any non-null pointer
        // is truthy and null-pointers are falsey, this is equivalent to a signature match.
        if ((... && std::get<Pointer<TRequiredComponents>>(entity_data->components)))
          // Cast the entity handle to the base handle type for systems to process them.
          callable(static_cast<typename Heap<smart>::Entity>(entity_data));
      }
      // Single-fire systems get a null-pointer as the entity handle.
    } else callable(typename Heap<smart>::Entity(nullptr)); // TODO: move check to runtime to avoid 0-reservation
  }

  // TODO: for_entities_with_parallel

private:
  /// Vector storing the entity metadata pointers.
  std::vector<Pointer<EntityData>> _entities;
};

// Convenience type alias of non-smart storage for passing the type into the scaffold ECS class.
template<typename... TComponents>
class ExplicitHeap : public Heap<false, TComponents...> {};

// Convenience type alias of smart storage for passing the type into the scaffold ECS class.
template<typename... TComponents>
class SmartHeap : public Heap<true, TComponents...> {};

}
