#pragma once

#include <type_traits>
#include <tuple>
#include <functional>
#include <mutex>

#include <taskflow/taskflow.hpp>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>

#include "ecs/scaffold/runtime.hpp"

#include "ecs/util/timer.hpp"
#include "ecs/util/to_hana_tuple_t.hpp"

namespace hana = boost::hana;
using namespace hana::literals;
namespace ct = boost::callable_traits;

namespace ecs::runtime {

/// Parallel ECS runtime, executing systems concurrently,
/// respecting a directed acyclic dependency graph.
///
/// @tparam TStorage The storage to be used.
/// @tparam TSystems The system types that are stored and executed. Usually inferred from the constructor.
template<template<typename...> typename TStorage, typename... TSystems>
class Parallel : ecs::Runtime<TStorage, TSystems...> {
public:
  /// The base runtime type to be inherited from.
  using Runtime = ecs::Runtime<TStorage, TSystems...>;

  /// Redeclaration of the type of this class itself as a type alias.
  /// Allows simpler usage further down.
  using ParallelRuntime = Parallel<TStorage, TSystems...>;

  /// The entity handle type from the storage.
  using typename Runtime::Entity;

  /// Runtime constructor.
  ///
  /// @param systems The systems to be executed. Will be moved in.
  Parallel(TSystems&&... systems) :
    // Systems are passed as references and stored in a runtime-owned tuple.
    _systems(std::make_tuple(std::forward<TSystems>(systems)...)),
    _runtime_manager(*this, _storage),
    _deferred_manager(*this, _storage)
  {
    // As is, lvalue-referenced systems would be copied in.
    // Copying in the systems is almost never what a user wants.
    // By forcing them to manually move in the systems, this behavior is made explicit.
    static_assert(
      (std::is_rvalue_reference_v<decltype(systems)> && ...),
      "Systems may only be moved in, not copied. Use std::move to transfer ownership or copy-construct beforehand."
    );
    // TODO: static assert invocability

    // Create a task for running each system. The result of this call is an std::tuple containing the tasks.
    auto graph = _taskflow.emplace([&]() { run_system<TSystems>(); } ...);

    // Iterate each system, and for each system all the systems after it by counting up from the current index to
    // the amount of systems specified exclusively (e.g. with 4 systems, 0: 1-2-3; 1: 2-3; 2: 3).
    // For each system pair (i.e., (0,1), (0,2), (0,3), (1,2), (1,3), (2,3)) determine whether a dependecy
    // exists between them, such that they may not run concurrently.
    // This is the case when either at least one of the systems writes to component data that the other also accesses
    // or when one system explicitly depends on the other by a system parameter.
    //
    // First element of the pair: count from 0 (inclusively) to the number of systems (exclusively).
    hana::for_each(hana::make_range(0_c, hana::size_c<sizeof...(TSystems) - 1>), [&](auto first_index) {
      // Second element of the pair: count from the first index (exclusively) to the number of systems (exclusively).
      hana::for_each(hana::make_range(first_index + hana::size_c<1>, hana::size_c<sizeof...(TSystems)>), [&](auto second_index) {
        // Get the system type of the first element.
        using FirstSystem = std::decay_t<std::tuple_element_t<first_index, std::tuple<TSystems...>>>;
        // Get the system type of the second element.
        using SecondSystem = std::decay_t<std::tuple_element_t<second_index, std::tuple<TSystems...>>>;
        // Since first_index != second_index, when the two systems are of the same type, they have both been passed into
        // the constructor. This is not supported, because it would make system dependencies ambiguous.
        static_assert(!std::is_same_v<FirstSystem, SecondSystem>, "Each system type may only be registered once.");

        // TODO: Document.
        // TODO: check component writes
        if constexpr (
          hana::contains(
            // The (system-type) parameters of the second system in decayed form.
            hana::transform(Info::template system_argtypes<SecondSystem>, hana::traits::decay),
            hana::type_c<FirstSystem>
          ) ||
          hana::contains(
            // The (system-type) parameters of the first system in decayed form.
            hana::transform(Info::template system_argtypes<FirstSystem>, hana::traits::decay),
            hana::type_c<SecondSystem>
          )
        ) {
          auto& first_task = std::get<first_index>(graph);
          auto& second_task = std::get<second_index>(graph);
          first_task.precede(second_task);
        }
      });
    });

    // ([&graph, task = std::get<type_index<TSystems, TSystems...>>(graph)]() {
    //   std::cout << task.name() << std::endl;
    //   Info::template system_argtypes<TSystems>
    // }(), ...);
  }

  /// Returns a reference to a stored system.
  template<typename TSystem>
  inline TSystem& get_system() {
    return std::get<TSystem>(_systems);
  }

  /// Defers an operation by queuing it.
  ///
  /// @param operation The operation to be deferred.
  inline void defer(auto&& operation) {
    // Lock the deferred operations vector to avoid concurrent writes.
    static std::mutex mutex;
    std::scoped_lock lock(mutex);
    _deferred_operations.push_back(operation);
  }

  /// Executes and clears all currently queued deferred operations.
  inline void dispatch_deferred_operations() {
    for (auto& operation : _deferred_operations) operation(_deferred_manager);
    _deferred_operations.clear();
  }

  /// Runs each system once.
  ///
  /// Systems with component dependencies are executed for each matching entities.
  /// Systems without dependencies are executed once only.
  void operator()() {
    // TODO: Only get delta_time if required by a system.
    // Get the time since the last call.
    _delta_time = _timer.reset();

    _executor.run(_taskflow).wait();

    // Execute all currently queued deferred operations.
    dispatch_deferred_operations();

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

  // The taskflow instance containing the dependency graph.
  tf::Taskflow _taskflow;

  // The taskflow executor used.
  tf::Executor _executor;

  // Runtime manager.
  using ParallelRuntimeManager = Runtime::template RuntimeManager<ParallelRuntime>;
  ParallelRuntimeManager _runtime_manager;

  // Deferred manager.
  using ParallelDeferredManager = Runtime::template DeferredManager<ParallelRuntime>;
  ParallelDeferredManager _deferred_manager;

  /// List of currently queued deferred operations.
  ///
  /// Deferring is just adding the operation to this vector.
  /// The operations are then executed at a later time (namely after all systems are run).
  std::vector<std::function<void(const ParallelDeferredManager&)>> _deferred_operations;

  // TODO: try average over time timer
  /// Timer for measuring frame times
  timing::Timer _timer;

  /// Field for temporarily storing the delta_time while systems execute.
  double _delta_time;

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
    /// @tparam parallel Whether to use inner parallelism or iterate sequentially.
    template<bool parallel>
    static auto run(auto& storage, auto&& callable) {
      if constexpr (parallel)
        return storage.template for_entities_with_parallel<TRequiredComponents...>(std::forward<decltype(callable)>(callable));
      else
        return storage.template for_entities_with<TRequiredComponents...>(std::forward<decltype(callable)>(callable));
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
  /// @tparam parallel Whether to use inner parallelism or iterate sequentially.
  template<bool parallel>
  auto for_entities_with(auto component_argtypes, auto&& callable) {
    // The properly templated `ForEntitiesWith` type.
    // Translates the types given by the `component_argtypes = boost::hana::tuple_t<Ts...>` to `ForEntitiesWith<Ts...>`.
    using Instance = typename decltype(hana::unpack(component_argtypes, hana::template_<ForEntitiesWith>))::type;
    return Instance::template run<parallel>(_storage, std::forward<decltype(callable)>(callable));
  }

  template<typename TSystem>
  void run_system() {
    // Extract the return type of the system call.
    // This is later used to determine whether a managed call needs to be done.
    using ReturnType = ct::return_type_t<TSystem>;
    // Iterate all entities with matching components associated with them.
    for_entities_with<Info::template parallelizable<TSystem>>(Info::template component_argtypes<TSystem>, [&](Entity entity) {
      // Transform the system-required parameter types to their filled-in values.
      // E.g., if a component type is to be passed in, this fetches that component.
      // This `args` tuple then contains the actual parameters to be passed into the system call.
      auto args = hana::transform(Info::template argtypes<TSystem>, [&](auto argtype) {
        // Fetch the argument type value from the tuple and convert to a type alias.
        using ArgType = typename decltype(argtype)::type;
        // Check if the argument type is a stored component type.
        if constexpr (hana::find(Info::components, argtype) != hana::nothing) {
          // Get a storage-stored component reference as the argument.
          // Plain references can not be stored in a heterogenous container.
          // Thus, reference_wrapper (created by std::ref) is needed to store the reference in the args container
          // (to later be unpacked into the system call).
          return std::ref(_storage.template get_component<ArgType>(entity));
        }
        // Check if the argument type is a stored system type.
        if constexpr (hana::find(Info::systems, argtype) != hana::nothing) {
        // Get a self-stored system reference as the argument.
          return std::ref(get_system<ArgType>());
        }
        // Check if the argument type is an entity handle.
        if constexpr (argtype == hana::type_c<Entity>) {
          return std::ref(entity);
        }
        // Check if the argument type is a floating point number, representing
        // a delta time (frametime / time since last frame).
        if constexpr (hana::find(hana::tuple_t<double, float>, argtype) != hana::nothing) {
          return  _delta_time;
        }
      });
      // If the system execution returns a callable operation, it is called immediately
      // with the runtime manager as an argument.
      // This is necessary, since the system functions can not be template functions
      // (because their parameter types are extracted and used here, requiring them to be concrete),
      // however, the runtime manager type is not known at the time of system declaration.
      // Thus, the system call may may return a template function (e.g., a lambda with `auto` parameter)
      // which is then instantiated with the correct manager type.
      if constexpr (std::is_invocable_v<ReturnType, const ParallelRuntimeManager&>) {
        // Call the system on the filled-in tuple of arguments and call the result with the manager.
        // `hana::unpack` applies the tuple of arguments as parameters to a function call.
        // The function called here is implicitly `system.operator()` for objects.
        hana::unpack(args, get_system<TSystem>())(_runtime_manager);
      } else {
        // If the system call result is not invocable, discard it.
        hana::unpack(args, get_system<TSystem>());
      }
    });
  }

public:

  /// Constant reference to the deferred manager.
  ///
  /// This allows for deferred operations to be done outside of actual system execution
  /// (e.g., in game initialization, loading levels, setting up configuration, etc.).
  const ParallelDeferredManager& manager = _deferred_manager;
};

}
