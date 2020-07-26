#pragma once

#include <type_traits>
#include <tuple>
#include <functional>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>

#include "runtime.hpp"
#include "../manager.hpp"

#include "../info.hpp"
#include "../util/timing/timer.hpp"

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
    // Call start method on all systems that implement it.
    hana::for_each(_systems, [&](auto& system) {
      if constexpr (SystemInfo<decltype(system)>::has_start) {
        // TODO: pass in deferred manager
        system.SYSTEM_START_METHOD();
      }
    });
  }

  // Runs each system once.
  void operator()() {
    double delta_time = _timer.reset();
    hana::for_each(_systems, [&](auto& system) {
      using SystemType = std::decay_t<decltype(system)>;
      using CallableType = decltype(&SystemType::SYSTEM_UPDATE_METHOD);
      using ReturnType = ct::return_type_t<CallableType>;

      // Lambda to call the system update on a set of arguments.
      auto call_update = hana::fuse([&](auto&&... args) { return system.SYSTEM_UPDATE_METHOD(args...); });

      constexpr auto argtypes = hana::transform(argtypes_of<CallableType>, hana::traits::decay);
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
              if constexpr (argtype == hana::type_c<Entity>) {
                return std::reference_wrapper(entity);
              } else {
                if constexpr (hana::find(hana::tuple_t<double, float>, argtype) != hana::nothing) {
                  return delta_time;
                } else {
                  // Use eval_if instead of constexpr if for conditional static_assert.
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
          _deferred_operations.push_back(call_update(args));
        } else {
          // Discard return value.
          call_update(args);
        }
      });
    });
    dispatch_deferred();
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

  // Container for deferred operations to be executed at a later time.
  std::vector<std::function<void(const SequentialDeferredManager&)>> _deferred_operations;

  // Dispatch deferred operations.
  inline void dispatch_deferred() {
    for (auto& operation : _deferred_operations) operation(_deferred_manager);
    _deferred_operations.clear();
  }

  // Timer for measuring frame times.
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

};

}