#pragma once

#include <type_traits>
#include <tuple>
#include <functional>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>

#include "../storage/storage.hpp"
#include "../manager.hpp"

#include "../util/callable_traits.hpp"
#include "../util/timing/timer.hpp"

namespace hana = boost::hana;
using namespace hana::literals;

namespace ecs::runtime {

template<template<typename...> typename TStorage, typename... TSystems>
requires Storage<TStorage>
class Sequential {
public:
  using Runtime = Sequential<TStorage, TSystems...>;
  using Entity = typename TStorage<>::Entity;

  // Systems are passed as references and stored in a runtime-owned tuple.
  Sequential(TSystems&&... systems) :
    _systems(std::make_tuple(std::forward<TSystems>(systems)...)),
    _runtime_manager(*this),
    _deferred_manager(*this)
  {}

  // Runs each system once.
  void operator()() {
    double delta_time = _timer.reset();
    std::vector<std::function<void(ecs::DeferredManager&)>> deferred;
    hana::for_each(_systems, [&](auto& system) {
      using ReturnType = ct::return_type_t<decltype(system)>;
      constexpr auto argtypes = hana::transform(argtypes_of<decltype(system)>, hana::traits::decay);
      constexpr auto component_argtypes = hana::intersection(hana::to_set(argtypes), component_types);
      for_entities_with(component_argtypes, [&](Entity entity) {
        auto args = hana::transform(argtypes, [&](auto argtype) {
          using ArgType = typename decltype(argtype)::type;
          // Check if the argument type is a stored component type:
          if constexpr (hana::find(component_types, argtype) != hana::nothing) {
            // reference_wrapper is needed to store the reference in the args container (to later be unpacked into the system call).
            return std::reference_wrapper(_storage.template get_component<ArgType>(entity));
          } else {
            // Check if the argument type is a stored system type:
            if constexpr (hana::find(system_types, argtype) != hana::nothing) {
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
        if constexpr (std::is_invocable_v<ReturnType, DeferredManager&>) {
          deferred.push_back(hana::unpack(args, system));
        } else {
          // Discard return value.
          hana::unpack(args, system);
        }
      });
    });
    for (auto& operation : deferred) operation(_deferred_manager);
  }

private:
  // System types in decayed form (removes cv-qualifiers and reference).
  static constexpr auto system_types = hana::transform(hana::tuple_t<TSystems...>, hana::traits::decay);
  // Set of component types used by any system in decayed form (removes cv-qualifiers and reference).
  // System-types and special types (Entity) are discarded.
  static constexpr auto component_types = hana::difference(
    hana::to_set(hana::transform(
      hana::flatten(
        hana::make_tuple(to_hana_tuple_t<ct::args_t<TSystems>>...)
      ),
      hana::traits::decay
    )),
    hana::union_(
      hana::to_set(system_types),
      hana::to_set(hana::tuple_t<
        Entity,
        double, float, // delta_time
        ecs::RuntimeManager
      >)
    )
  );

  // Tuple to store references to the systems. (std::tuple instead of hana::tuple for std::get<> via type).
  std::tuple<std::decay_t<TSystems>...> _systems;
  // The type of Storage used, determined by applying the associated component types as TStorage<...> template-parameters.
  using Storage = typename decltype(hana::unpack(component_types, hana::template_<TStorage>))::type;
  Storage _storage;

  class SequentialRuntimeManager : public virtual ecs::RuntimeManager {
  public:
    SequentialRuntimeManager(Runtime& runtime) : _runtime(runtime) {}

    size_t get_entity_count() override {
      return _runtime._storage.get_size();
    }
  protected:
    Runtime& _runtime;
  } _runtime_manager;

  class SequentialDeferredManager : public ecs::DeferredManager, public SequentialRuntimeManager {
  public:
    SequentialDeferredManager(Runtime& runtime) : SequentialRuntimeManager(runtime) {}

    using SequentialRuntimeManager::get_entity_count;

    void spawn_entity() override {
      _runtime._storage.new_entity();
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

};

}