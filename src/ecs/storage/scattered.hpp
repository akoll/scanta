#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <unordered_set>
#include <memory>
#include <type_traits>


#include <boost/hana.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

namespace ecs::storage {

  /// Internal namespace only used in this header.
  namespace internal {

  /// Structure for setting and accessing scattered storage configuration.
  ///
  /// Allows the configuration to be passed in as a compile-time template
  /// parameter instead of a preprocessor define.
  /// This struct is only ever instantiated at compile-time.
  constexpr struct ScatteredOptions {
    /// Whether to use [smart pointers](https://en.cppreference.com/book/intro/smart_pointers) or regular pointers.
    const bool smart_pointers = false;
    /// Whether to use a set or a vector for storing entity metadata.
    const bool entity_set = false;

    /// Copies the options but with smart pointers configured.
    consteval ScatteredOptions use_smart_pointers() const {
      return ScatteredOptions{true, this->entity_set};
    }

    /// Copies the options but with entity set configured.
    consteval ScatteredOptions use_entity_set() const {
      return ScatteredOptions{this->smart_pointers, true};
    }
  } scattered_options;

  /// Base declaration for partial specialization.
  template<ScatteredOptions options, typename... TStoredComponents>
  class Scattered;

  /// Partial specialization for the case of no stored components.
  ///
  /// This is required because an entity handle type needs to be exposed
  /// to the scaffold before the list of stored components is known.
  /// @tparam options The scattered storage options to be used.
  template<ScatteredOptions options>
  class Scattered<options> {
  public:
    /// The base entity handle type of the scattered storage.
    ///
    /// This entity handle type is used for the declaration of systems. A scattered storage instance with stored component types specified
    /// defines another entity handle type, which can be converted from and to this one.
    /// A void-pointer is wrapped as the handle type, since the underlying entity metadata type is not known without component types.
    class Entity {
      // All other Scattered classes are friends, so they can access the void-pointer for casting.
      template<ScatteredOptions, typename...>
      friend class Scattered;
    private:
      /// The void-pointer type for this storage.
      ///
      /// If smart pointers are used, uses a shared pointer, otherwise a regular pointer.
      using VoidPointer = typename std::conditional<options.smart_pointers, std::shared_ptr<void>, void*>::type;
      VoidPointer _pointer;
    public:
      /// Default constructor.
      Entity() = default;

      /// Constructor for converting from plain void-pointers.
      Entity(VoidPointer pointer) : _pointer(pointer) {}
    };

    template<typename... TRequiredComponents>
    void for_entities_with(auto&& callable) const {
      if (sizeof...(TRequiredComponents) == 0)
        callable(Entity(nullptr));
    }

    template<typename... TRequiredComponents>
    void for_entities_with_parallel(auto&& callable) const {
      if (sizeof...(TRequiredComponents) == 0)
        callable(Entity(nullptr));
    }

    void new_entity(auto&&...) const {}
  };

  /// Stores components and entity metadata in dynamically allocated and scattered heap regions.
  ///
  /// @tparam options The scattered storage options to be used.
  /// @tparam TStoredComponents The component types to be stored.
  template<ScatteredOptions options, typename... TStoredComponents>
  class Scattered : Scattered<options> {
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
    using Pointer = typename std::conditional<options.smart_pointers, std::shared_ptr<T>, T*>::type;
  public:
    /// Entity metadata used by the storage internally.
    struct EntityMetadata {
      /// The associated components are referenced in a tuple of pointers.
      /// For each component type, a pointer is stored, pointing to the component data.
      /// When no component of a type is attached to the entity, that pointer is null.
      std::tuple<Pointer<TStoredComponents>...> components;

      /// Deletes all components associated with this entity.
      void clear_components() {
        // If regular pointers are used, memory needs to be freed first using `delete`.
        if constexpr (!options.smart_pointers)
          ((delete std::get<Pointer<std::decay_t<TStoredComponents>>>(components)), ...);
        // Set all pointer to null using a fold-expression.
        ((std::get<Pointer<std::decay_t<TStoredComponents>>>(components) = nullptr), ...);
      }

      /// Destructor which deletes all components.
      ~EntityMetadata() {
        if constexpr (!options.smart_pointers)
          ((delete std::get<Pointer<std::decay_t<TStoredComponents>>>(components)), ...);
      }
    };

    /// The entity handle type of the scattered storage.
    ///
    /// This entity handle type encapsulates the entity metadata with respect to the stored component types.
    /// In system definitions the base storage handle type is used. Both handle types are implicitly convertible.
    class Entity {
    private:
      /// The underlying metadata pointer.
      Pointer<EntityMetadata> _pointer;
    public:
      /// Constructor for converting from plain pointers.
      Entity(Pointer<EntityMetadata> pointer) : _pointer(pointer) {}

      /// Constructor for converting from the base entity handle type (used in system definitions).
      Entity(typename Scattered<options /* no stored component types */>::Entity other) {
        // Cast the void-pointer in the base handle to the appropriate handle from this storage.
        if constexpr (options.smart_pointers)
          _pointer = std::static_pointer_cast<EntityMetadata>(other._pointer);
        else
          _pointer = static_cast<EntityMetadata*>(other._pointer);
      }

      /// Conversion operator for converting to a base handle.
      operator typename Scattered<options>::Entity() {
        // Construct a base handle from the metadata handle.
        if constexpr (options.smart_pointers)
          return std::static_pointer_cast<Scattered<options>::Entity>(_pointer);
        else
          return static_cast<typename Scattered<options>::Entity>(_pointer);
      }

      /// Conversion operator for converting to a plain pointer.
      operator Pointer<EntityMetadata>() {
        return _pointer;
      }

      /// Dereferencing pointer access operator.
      Pointer<EntityMetadata> operator->() {
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
    /// Any components previously attached to the entity and not passed in again are detached.
    /// The passed in components make up the new complete set of components attached to that entity.
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
      if constexpr (options.smart_pointers)
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
      Pointer<EntityMetadata> entity_data;
      // Construct either shared or plain.
      if constexpr (options.smart_pointers)
        entity_data = std::make_shared<EntityMetadata>();
      else
        entity_data = new EntityMetadata();
      if constexpr (!options.entity_set) {
        // Add the new metadata pointer to the entity vector. This is O(1).
        _entities.push_back(entity_data);
      } else {
        // Add the new metadata pointer to the entity set. This is worst-case O(N).
        _entities.insert(entity_data);
      }
      // Set initial components by forwarding them (retaining references without copy).
      set_components(entity_data, std::forward<decltype(components)>(components)...);
    }

    /// Removes an entity from the storage.
    ///
    /// Frees any memory for the entity metadata and associated components.
    void remove_entity(Entity entity) {
      if constexpr (!options.entity_set) {
        // Find the entity in the vector of pointers. This is O(N).
        auto it = std::find(_entities.begin(), _entities.end(), static_cast<Pointer<EntityMetadata>>(entity));
        // If the entity has been found (/ is stored).
        if (it != _entities.end())
          // Remove it from the entity vector.
          _entities.erase(it);
        // TODO: else: entity not found
      } else {
        // Remove the entity from the map. This is on average O(1).
        _entities.erase(entity);
      }
      // Delete the entity. This also deletes all components.
      if constexpr (!options.smart_pointers) delete entity;
    }

    /// Execute a callable on each entity with all required components attached.
    ///
    /// @tparam TRequiredComponents The set of component types required to be attached to an entity to be processed.
    /// @param callable The callable to be executed with each matched entity's index as an argument.
    template<typename... TRequiredComponents>
    void for_entities_with(auto&& callable) const {
      /// If the list of required component types is empty, the callable is called exactly once.
      if constexpr (sizeof...(TRequiredComponents) > 0) {
        // TODO: static_assert component types handled
        // Iterate all stored entities.
        for (Pointer<EntityMetadata> entity_data : _entities) {
          // Check the entity signature by seeing if all required component pointers are non-null.
          // This is done using a fold-expression with the boolean AND operator. Since any non-null pointer
          // is truthy and null-pointers are falsey, this is equivalent to a signature match.
          if ((... && std::get<Pointer<TRequiredComponents>>(entity_data->components)))
            // Cast the entity handle to the base handle type for systems to process them.
            callable(static_cast<typename Scattered<options>::Entity>(entity_data));
        }
        // Single-fire systems get a null-pointer as the entity handle.
      } else callable(typename Scattered<options>::Entity(nullptr)); // TODO: move check to runtime to avoid 0-reservation
    }

    // TODO: document
    template<typename... TRequiredComponents>
    void for_entities_with_parallel(auto&& callable) const {
      /// If the list of required component types is empty, the callable is called exactly once.
      if constexpr (sizeof...(TRequiredComponents) > 0) {
        // TODO: static_assert component types handled
        // Iterate all stored entities.
        if constexpr (!options.entity_set) {
          #pragma omp parallel for
          for (Pointer<EntityMetadata> entity_data : _entities) {
            // Check the entity signature by seeing if all required component pointers are non-null.
            // This is done using a fold-expression with the boolean AND operator. Since any non-null pointer
            // is truthy and null-pointers are falsey, this is equivalent to a signature match.
            if ((... && std::get<Pointer<TRequiredComponents>>(entity_data->components)))
              // Cast the entity handle to the base handle type for systems to process them.
              callable(static_cast<typename Scattered<options>::Entity>(entity_data));
          }
        } else {
          // Iterate all buckets in the set in parallel.
          #pragma omp parallel for
          for (size_t bucket = 0; bucket < _entities.bucket_count(); ++bucket) {
            // Iterate all stored entities in the bucket.
            for (auto it = _entities.begin(bucket); it != _entities.end(bucket); ++it) {
              Pointer<EntityMetadata> entity = *it;
              // Check the entity signature by seeing if all required component pointers are non-null.
              // This is done using a fold-expression with the boolean AND operator. Since any non-null pointer
              // is truthy and null-pointers are falsey, this is equivalent to a signature match.
              if ((... && std::get<Pointer<TRequiredComponents>>(entity->components)))
                // Cast the entity handle to the base handle type for systems to process them.
                callable(static_cast<typename Scattered<options>::Entity>(entity));
            }
          }
        }
        // Single-fire systems get a null-pointer as the entity handle.
      } else callable(typename Scattered<options>::Entity(nullptr)); // TODO: move check to runtime to avoid 0-reservation
    }

  private:
    /// Entity metadata storage.
    ///
    /// Depending on the storage options `entity_set`, either a vector or a set of pointers.
    std::conditional<
      options.entity_set,
      std::unordered_set<Pointer<EntityMetadata>>,
      std::vector<Pointer<EntityMetadata>>
    >::type _entities;
  };

  /// Scattered storage configuration class.
  template<ScatteredOptions options = scattered_options>
  class ScatteredCustom {
  public:
    /// The configured storage.
    template<typename... TComponents>
    using Storage = internal::Scattered<options, TComponents...>;

    /// This class but with smart pointers configured.
    using WithSmartPointers = ScatteredCustom<options.use_smart_pointers()>;
    /// This class but with entity set configured.
    using WithEntitySet = ScatteredCustom<options.use_entity_set()>;
  };

  }

/// Scattered storage with custom options.
///
/// This avoids having to write `<>` after CustomScattered when using.
using ScatteredCustom = internal::ScatteredCustom<>;

/// Scattered storage with default options.
template<typename... TComponents>
using Scattered = internal::Scattered<internal::scattered_options, TComponents...>;

}
