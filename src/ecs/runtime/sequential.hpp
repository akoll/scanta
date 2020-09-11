#pragma once

#include <type_traits>
#include <tuple>
#include <functional>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>

#include "ecs/scaffold/runtime.hpp"
#include "ecs/scaffold/manager.hpp"

#include "ecs/util/timer.hpp"

namespace hana = boost::hana;

namespace ecs::runtime {

template<template<typename...> typename TStorage, typename... TSystems>
class Sequential : ecs::Runtime<TStorage, TSystems...> {
public:
  using Runtime = ecs::Runtime<TStorage, TSystems...>;
  using SequentialRuntime = Sequential<TStorage, TSystems...>;

  using typename Runtime::Entity;

  // Systems are passed as references and stored in a runtime-owned tuple.
  Sequential(TSystems&&... systems) :
    _systems(std::make_tuple(std::forward<TSystems>(systems)...)),
    _runtime_manager(*this),
    _deferred_manager(*this)
  {
    static_assert(
      (std::is_rvalue_reference_v<decltype(systems)> && ...),
      "Systems may only be moved in, not copied. Use std::move to transfer ownership or copy-construct beforehand."
    );
    // TODO: static assert invocability
  }

  template<typename TSystem>
  TSystem& get_system() {
    return std::get<TSystem>(_systems);
  }

  // Runs each system once.
  void operator()() {
    double delta_time = _timer.reset();
    std::vector<std::function<void(const SequentialDeferredManager&)>> deferred;
    hana::for_each(_systems, [&](auto& system) {
      using ReturnType = ct::return_type_t<decltype(system)>;
      constexpr auto argtypes = hana::transform(argtypes_of<decltype(system)>, hana::traits::decay);
      constexpr auto component_argtypes = hana::intersection(hana::to_set(argtypes), Info::components);
      for_entities_with(component_argtypes, [&](Entity entity) {
        auto args = hana::transform(argtypes, [&](auto argtype) {
          using ArgType = typename decltype(argtype)::type;
          // Check if the argument type is a stored component type:
          if constexpr (hana::find(Info::components, argtype) != hana::nothing) {
            // reference_wrapper is needed to store the reference in the args container (to later be unpacked into the system call).
            return std::reference_wrapper(_storage.template get_component<ArgType>(entity));
          } else {
            // Check if the argument type is a stored system type:
            if constexpr (hana::find(Info::systems, argtype) != hana::nothing) {
              return std::reference_wrapper(std::get<ArgType>(_systems));
            } else {
              // Use eval_if instead of constexpr if for conditional static_assert.
              // TODO: use hana switch
              if constexpr (argtype == hana::type_c<Entity>) {
                return std::reference_wrapper(entity);
              } else {
                if constexpr (hana::find(hana::tuple_t<double, float>, argtype) != hana::nothing) {
                  return delta_time;
                } else {
                  return hana::eval_if(
                    argtype == hana::type_c<ecs::RuntimeManager>,
                    [&](auto _) { return _runtime_manager; },
                    [&](auto _) { static_assert(_(false), "System argument types are invalid"); }
                  );
                }
              }
            }
          }
        });
        if constexpr (std::is_invocable_v<ReturnType, const SequentialDeferredManager&>) {
          deferred.push_back(hana::unpack(args, system));
        } else {
          // Discard return value.
          hana::unpack(args, system);
        }
      });
    });

    for (auto& operation : deferred) operation(_deferred_manager);

    // TODO: Is this necessary every frame? Is this necessary for every storage? Sfinae if not defined.
    if constexpr (requires { _storage.refresh(); })
      _storage.refresh();
  }

private:
  using typename Runtime::Info;

  // Tuple to store references to the systems. (std::tuple instead of hana::tuple for std::get<> via type).
  std::tuple<std::decay_t<TSystems>...> _systems;

  using Runtime::_storage;

  class SequentialRuntimeManager : public virtual ecs::RuntimeManager {
  public:
    SequentialRuntimeManager(SequentialRuntime& runtime) : _runtime(runtime) {}

    size_t get_entity_count() const override {
      return _runtime._storage.get_size();
    }
  protected:
    SequentialRuntime& _runtime;
  } _runtime_manager;

  class SequentialDeferredManager : public SequentialRuntimeManager {
  public:
    SequentialDeferredManager(SequentialRuntime& runtime) : SequentialRuntimeManager(runtime) {}

    using SequentialRuntimeManager::get_entity_count;

    void new_entity(auto&&... components) const {
      _runtime._storage.new_entity(std::forward<decltype(components)>(components)...);
    }

    void remove_entity(Entity entity) const {
      _runtime._storage.remove_entity(entity);
    }
  protected:
    using SequentialRuntimeManager::_runtime;
  } _deferred_manager;

  // Timer for measuring frame times
  // TODO: try average over time timer
  timing::Timer _timer;

  // Helper class for easier hana::template_ unpacking.
  template<typename... TRequiredComponents>
  struct ForEntitiesWith {
    static auto run(auto& storage, auto&& callable) {
      return storage.template for_entities_with<TRequiredComponents...>(callable);
    }
  };

  // Helper function wrapping ForEntitiesWith instantiation.
  auto for_entities_with(auto component_argtypes, auto&& callable) {
    using Instance = typename decltype(hana::unpack(component_argtypes, hana::template_<ForEntitiesWith>))::type;
    return Instance::run(_storage, callable);
  }

public:

  /// Constant reference to the deferred manager.
  ///
  /// This allows for deferred operations to be done outside of actual system execution
  /// (e.g., in game initialization, loading levels, setting up configuration).
  const SequentialDeferredManager& manager = _deferred_manager;
};

}
