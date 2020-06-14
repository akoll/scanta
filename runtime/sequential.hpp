#pragma once

#include <type_traits>
#include <tuple>
#include <functional>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>

#include "../storage/storage.hpp"

#include "../util/callable_traits.hpp"

namespace hana = boost::hana;
using namespace hana::literals;

namespace ecs::runtime {

template<template<typename...> typename TStorage, typename... TSystems>
requires Storage<TStorage>
class Sequential {
public:
  using Entity = typename TStorage<>::Entity;

  // Systems are default-constructed and stored in a runtime-owned tuple.
  Sequential(): _systems(std::make_tuple(TSystems{}...)) {}

  // Runs each system once.
  void operator()() {
    hana::for_each(_systems, [this](auto& system) {
      constexpr auto argtypes = hana::transform(argtypes_of<decltype(system)>, hana::traits::decay);
      constexpr auto component_argtypes = hana::intersection(hana::to_set(argtypes), component_types);
      (typename decltype(hana::unpack(component_argtypes, hana::template_<ForEntitiesWith>))::type){}(
        _storage, [&](Entity entity) {
          auto args = hana::transform(argtypes, [this, &entity](auto argtype) {
            using ArgType = typename decltype(argtype)::type;
            return hana::eval_if(
              // Check if the argument type is a stored component type:
              hana::find(component_types, argtype) != hana::nothing,
              [&](auto _) {
                // reference_wrapper is needed to store the reference in the args container (to later be unpacked into the system call).
                return std::reference_wrapper(_storage.template get_component<ArgType>(entity));
              },
              [&](auto _) { return hana::eval_if(
                // Check if the argument type is a stored system type:
                hana::find(system_types, argtype) != hana::nothing,
                [&](auto _) { return std::reference_wrapper(std::get<ArgType>(_(_systems))); },
                [&](auto _) { return hana::eval_if(
                  argtype == hana::type_c<Entity>,
                  [&](auto _) { return std::reference_wrapper(entity); },
                  [&](auto _) { static_assert(_(false), "System argument types are invalid"); }
                ); }
              ); }
            );
          });
          return hana::unpack(args, system);
        }
      );
    });
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
      hana::to_set(hana::tuple_t<Entity>)
    )
  );

  // Tuple to store references to the systems. (std::tuple instead of hana::tuple for std::get<> via type).
  std::tuple<std::decay_t<TSystems>...> _systems;
  // The type of Storage used, determined by applying the associated component types as TStorage<...> template-parameters.
  using Storage = typename decltype(hana::unpack(component_types, hana::template_<TStorage>))::type;
  Storage _storage;

  // Helper struct wrapping storage.for_entities_with. Required to be used for a hana::template_ as an unpack target.
  template<typename... TRequiredComponents>
  struct ForEntitiesWith {
    auto operator()(auto& storage, auto callable) {
      return storage.template for_entities_with<TRequiredComponents...>(callable);
    }
  };


};

}