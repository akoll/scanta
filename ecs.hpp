#pragma once

#include "storage/storage.hpp"

#include "util/to_hana_tuple.hpp"

#include <boost/hana.hpp>
#include <boost/callable_traits.hpp>
#include <boost/mp11.hpp>

namespace hana = boost::hana;
namespace ct = boost::callable_traits;
using namespace hana::literals;

namespace ecs {

// template<template<typename...> typename TStorage>
// requires Storage<TStorage>
Storage{TStorage}
struct EntityComponentSystem {

  template<typename... TSystems>
  class Runtime {
  public:
    // Systems are passed as lvalue-references and stored as referece_wrapper.
    Runtime(TSystems&... systems): _systems(hana::make_tuple(std::reference_wrapper(systems)...)) {};

    void operator()() {
      hana::for_each(_systems, [this](auto& system) {
        auto args = hana::transform(get_argtypes(system), [this](auto argtype) {
          using ComponentType = typename decltype(hana::traits::decay(argtype))::type;
          // reference_wrapper is needed to store the reference in the args container (to later be unpacked into the system call).
          return std::reference_wrapper(_storage.template getComponent<ComponentType>(0));
        });
        hana::unpack(args, system);
      });
    }
  private:
    // System types in decayed form (removes cv-qualifiers and reference).
    static constexpr auto system_types = hana::transform(hana::tuple_t<TSystems...>, hana::traits::decay);

    // Set of component types used by any system in decayed form (removes cv-qualifiers and reference).
    static constexpr auto component_types = hana::difference(
      hana::to_set(hana::transform(
        hana::flatten(hana::make_tuple(to_hana_tuple<ct::args_t<TSystems>>...)),
        hana::traits::decay
      )),
      system_types
    );

    // The type of Storage used, determined by applying the associated component types as TStorage<...> template-parameters.
    using Storage = typename decltype(hana::unpack(component_types, hana::template_<TStorage>))::type;

    // Tuple to store references to the systems.
    hana::tuple<std::reference_wrapper<TSystems>...> _systems;
    // Component Storage.
    Storage _storage;
  };

};

}