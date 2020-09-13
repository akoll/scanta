#pragma once

#include <type_traits>
#include <tuple>
#include <functional>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>

#include "ecs/scaffold/runtime.hpp"

#include "ecs/util/timer.hpp"

namespace hana = boost::hana;

namespace ecs::runtime {

/// Sequential ECS runtime, executing systems one after the other.
///
/// @tparam TStorage The storage to be used.
/// @tparam TSystems The system types that are stored and executed. Usually inferred from the constructor.
template<template<typename...> typename TStorage, typename... TSystems>
class Sequential : ecs::Runtime<TStorage, TSystems...> {
public:
  /// The base runtime type to be inherited from.
  using Runtime = ecs::Runtime<TStorage, TSystems...>;

  /// Redeclaration of the type of this class itself as a type alias.
  /// Allows simpler usage further down.
  using SequentialRuntime = Sequential<TStorage, TSystems...>;

  /// The entity handle type from the storage.
  using typename Runtime::Entity;

  /// Runtime constructor.
  ///
  /// @param systems The systems to be executed. Will be moved in.
  Sequential(TSystems&&... systems) :
    // Systems are passed as references and stored in a runtime-owned tuple.
    _systems(std::make_tuple(std::forward<TSystems>(systems)...)),
    _runtime_manager(*this),
    _deferred_manager(*this)
  {
    // As is, lvalue-referenced systems would be copied in.
    // Copying in the systems is almost never what a user wants.
    // By forcing them to manually move in the systems, this behavior is made explicit.
    static_assert(
      (std::is_rvalue_reference_v<decltype(systems)> && ...),
      "Systems may only be moved in, not copied. Use std::move to transfer ownership or copy-construct beforehand."
    );
    // TODO: static assert invocability
  }

  /// Returns a reference to a stored system.
  template<typename TSystem>
  TSystem& get_system() {
    return std::get<TSystem>(_systems);
  }

  /// Runs each system once.
  ///
  /// Systems with component dependencies are executed for each matching entities.
  /// Systems without dependencies are executed once only.
  void operator()() {
    // Get the time since the last call.
    double delta_time = _timer.reset();
    // Iterate each system stored.
    hana::for_each(_systems, [&](auto& system) {
      // Extract the return type of the system call.
      // This is later used to determine whether a managed call needs to be done.
      using ReturnType = ct::return_type_t<decltype(system)>;
      // Get the parameters required by the system.
      constexpr auto argtypes = hana::transform(argtypes_of<decltype(system)>, hana::traits::decay);
      // Filter out just the parameters that are stored component types.
      constexpr auto component_argtypes = hana::intersection(hana::to_set(argtypes), Info::components);
      // Iterate all entities with such components associated with them.
      for_entities_with(component_argtypes, [&](Entity entity) {
        // Transform the system-required parameter types to their filled-in values.
        // E.g., if a component type is to be passed in, this fetches that component.
        // This `args` tuple then contains the actual parameters to be passed into the system call.
        auto args = hana::transform(argtypes, [&](auto argtype) {
          // Fetch the argument type value from the tuple and convert to a typename.
          using ArgType = typename decltype(argtype)::type;
          // Check if the argument type is a stored component type.
          if constexpr (hana::find(Info::components, argtype) != hana::nothing) {
            // Get a storage-stored component reference as the argument.
            // Plain references can not be stored in a heterogenous container.
            // This, reference_wrapper is needed to store the reference in the args container
            // (to later be unpacked into the system call).
            return std::reference_wrapper(_storage.template get_component<ArgType>(entity));
          } else {
            // Check if the argument type is a stored system type.
            if constexpr (hana::find(Info::systems, argtype) != hana::nothing) {
            // Get a self-stored system reference as the argument.
              return std::reference_wrapper(std::get<ArgType>(_systems));
            } else {
              // Check if the argument type is an entity handle.
              if constexpr (argtype == hana::type_c<Entity>) {
                return std::reference_wrapper(entity);
              } else {
                // Check if the argument type is a floating point number, representing
                // a delta time (frametime / time since last frame).
                // `eval_if` is used here instead of a `if constexpr` because it supports
                // conditional `static_assert` for a custom compile-time failure.
                return hana::eval_if(
                  (hana::find(hana::tuple_t<double, float>, argtype) != hana::nothing),
                  [&](auto _) { return _(delta_time); } ,
                  [&](auto _) { static_assert(_(false), "System argument types are invalid"); }
                );
              }
            }
          }
        });
        // If the system execution returns a callable operation, it is called immediately
        // with the runtime manager as an argument.
        // This is necessary, since the system functions can not be template functions
        // (because their parameter types are extracted and used here, requiring them to be concrete),
        // however, the runtime manager type is not known at the time of system declaration.
        // Thus, the system call may may return a template function (e.g., a lambda with `auto` parameter)
        // which is then instantiated with the correct manager type.
        if constexpr (std::is_invocable_v<ReturnType, const SequentialRuntimeManager&>) {
          // Call the system on the filled-in tuple of arguments and call the result with the manager.
          // `hana::unpack` applies the tuple of arguments as parameters to a function call.
          // The function called here is implicitly `system.operator()` for objects.
          hana::unpack(args, system)(_runtime_manager);
        } else {
          // If the system call result is not invocable, discard it.
          hana::unpack(args, system);
        }
      });
    });

    // Execute all currently queued deferred operations.
    for (auto& operation : _deferred_operations) operation(_deferred_manager);
    _deferred_operations.clear();

    // Certain entity-component storages need to be refreshed periodically to restore
    // certain preconditions or optimizations.
    // Check whether the used storage supports it, by seeing if the expression
    // is well-formed. If so, call it.
    if constexpr (requires { _storage.refresh(); })
      _storage.refresh();
  }

private:
  /// Shortening type alias to access info more easily.
  using typename Runtime::Info;

  /// The entity & component storage.
  typename Runtime::Storage _storage;

  /// Tuple to store references to the systems.
  ///
  /// This tuple is iterated at execution time by `boost::hana` functions.
  /// This could be a `hana::tuple`, however, those do not support getting
  /// elements by their type. `std::get` does however, and so a std::tuple
  /// is used here. Conversion functions to a `hana::tuple` exist, enabling this.
  std::tuple<std::decay_t<TSystems>...> _systems;

  /// The runtime manager to be passed into system executions.
  ///
  /// Systems may need to be able to execute certain runtime operations
  /// and access the underlying storage. Therefor, a _manager_ is provided
  /// to the system execution, on which such operations can be done.
  class SequentialRuntimeManager {
  public:
    /// Constructs a manager for some sequential runtime.
    ///
    /// @param runtime The runtime to be managed.
    SequentialRuntimeManager(SequentialRuntime& runtime) : _runtime(runtime) {}

    // Immediately executed functions:

    /// Returns the size of the underlying entity storage.
    ///
    /// Note that depending on the storage implementation, this might also be
    /// just an upper bound when called non-deferred.
    size_t get_entity_count() const {
      return _runtime._storage.get_size();
    }

    /// Defers an operation by queuing it with the runtime.
    ///
    /// @param operation The operation to be deferred.
    void defer(auto&& operation) const {
      _runtime._deferred_operations.push_back(operation);
    }

    // Deferred functions:

    /// Creates a new entity in the scene.
    ///
    /// When called, the entity is not created immediately,
    /// but merely queued as a deferred operation.
    /// This is done for consistent predictability when interchanging
    /// with parallelized runtimes, since those use deferring to avoid
    /// mutex violations and race conditions.
    ///
    /// @param components The set of components to be initially associated with the new entity.
    void new_entity(auto&&... components) const {
      defer([&](const auto& manager) {
        manager.new_entity(std::forward<decltype(components)>(components)...);
      });
    }

    /// Removes an entity from the scene.
    ///
    /// When called, the entity is not removed immediately,
    /// but merely queued as a deferred operation.
    /// This is done for consistent predictability when interchanging
    /// with parallelized runtimes, since those use deferring to avoid
    /// mutex violations and race conditions.
    ///
    /// @param entity The entity to be removed.
    void remove_entity(Entity entity) const {
      defer([entity](const auto& manager) {
        manager.remove_entity(entity);
      });
    }
  protected:
    /// The runtime to be managed by this manager.
    SequentialRuntime& _runtime;
  } _runtime_manager;

  /// The deferred manager to be passed into deferred operation executions.
  ///
  /// Deferred operations may need to be able to execute certain runtime operations
  /// and access the underlying storage. Therefor, a _manager_ is provided
  /// to the deferred call, on which such operations can be done.
  ///
  /// This manager's methods are a superset of the runtime manager's.
  class SequentialDeferredManager : public SequentialRuntimeManager {
  public:
    /// Constructs a manager for some sequential runtime.
    ///
    /// @param runtime The runtime to be managed.
    SequentialDeferredManager(SequentialRuntime& runtime) : SequentialRuntimeManager(runtime) {}

    // Include runtime manager functionality.
    using SequentialRuntimeManager::get_entity_count;

    /// Creates a new entity in the scene.
    ///
    /// @param components The set of components to be initially associated with the new entity.
    void new_entity(auto&&... components) const {
      _runtime._storage.new_entity(std::forward<decltype(components)>(components)...);
    }

    /// Removes an entity from the scene.
    ///
    /// @param entity The entity to be removed.
    void remove_entity(Entity entity) const {
      _runtime._storage.remove_entity(entity);
    }
  protected:
    /// The runtime to be managed by this manager.
    using SequentialRuntimeManager::_runtime;
  } _deferred_manager;

  /// List of currently queued deferred operations.
  ///
  /// Deferring is just adding the operation to this vector.
  /// The operations are then executed at a later time (namely after all systems are run).
  std::vector<std::function<void(const SequentialDeferredManager&)>> _deferred_operations;

  // TODO: try average over time timer
  /// Timer for measuring frame times
  timing::Timer _timer;

  /// Struct encapsulating a function call to iterate over entities in the storage
  /// with a set of required components.
  ///
  /// This only exists because `hana::template_` does not work for functions directly.
  /// @tparam TRequiredComponents The components to be required on the entity.
  template<typename... TRequiredComponents>
  struct ForEntitiesWith {
    /// Executes an entity iteration with a callable on some storage.
    ///
    /// @param storage The storage to be accessed.
    /// @param callable The operation to be executed for each matching entity.
    static auto run(auto& storage, auto&& callable) {
      return storage.template for_entities_with<TRequiredComponents...>(callable);
    }
  };

  /// Executes an entity iteration with a set of required components.
  ///
  /// This is a helper function wrapping ForEntitiesWith instantiation.
  /// The reasoning behind this is, that the storage-defined for_entities_with function
  /// requires type as template parameters, while the `boost::hana` functions
  /// return the types as values. This function takes in the required component types
  /// as a `boost::hana::tuple_t` and translates them to the corresponding template call.
  /// @param component_argtypes The required component types matched against each entity as a `boost::hana::tuple_t`.
  /// @param callable The operation to be executed for each matching entity.
  auto for_entities_with(auto component_argtypes, auto&& callable) {
    // The properly templated `ForEntitiesWith` type.
    // Translates the types given by the `component_argtypes = boost::hana::tuple_t<Ts...>` to `ForEntitiesWith<Ts...>`.
    using Instance = typename decltype(hana::unpack(component_argtypes, hana::template_<ForEntitiesWith>))::type;
    return Instance::run(_storage, callable);
  }

public:

  /// Constant reference to the deferred manager.
  ///
  /// This allows for deferred operations to be done outside of actual system execution
  /// (e.g., in game initialization, loading levels, setting up configuration, etc.).
  const SequentialDeferredManager& manager = _deferred_manager;
};

}
