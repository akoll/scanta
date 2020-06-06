#pragma once

#include "storage/storage.hpp"

#include "util/to_hana_tuple_t.hpp"

#include <type_traits>
#include <tuple>
// For std::reference_wrapper.
#include <functional>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>
#include <boost/callable_traits.hpp>

namespace hana = boost::hana;
namespace ct = boost::callable_traits;
using namespace hana::literals;

namespace ecs {

template<typename T>
constexpr auto get_argtypes(const std::reference_wrapper<T>& system) {
  return to_hana_tuple_t<ct::args_t<T>>;
}

Storage{TStorage}
struct EntityComponentSystem {

  template<typename... TSystems>
  class SequentialRuntime {
  public:
    // Systems are passed as lvalue-references and stored as referece_wrapper.
    SequentialRuntime(TSystems&... systems): _systems(std::make_tuple(std::reference_wrapper(systems)...)) {};

    // Runs each system once.
    void operator()() {
      hana::for_each(_systems, [this](auto& system) {
        static_assert(
          hana::type_c<ct::return_type_t<decltype(system)>> == hana::type_c<void>
          || std::is_invocable_v<ct::return_type_t<decltype(system)>>,
          "System return type is neither void nor invokable"
        );

        auto args = hana::transform(hana::transform(get_argtypes(system), hana::traits::decay), [this](auto argtype) {
          using ArgType = typename decltype(argtype)::type;
          return hana::eval_if(hana::find(component_types, argtype) != hana::nothing,
            // The argument type is a stored component type:
            [&](auto _) {
              // reference_wrapper is needed to store the reference in the args container (to later be unpacked into the system call).
              return std::reference_wrapper(_storage.template getComponent<ArgType>(0));
            },
            [&](auto _) {
              return hana::eval_if(hana::find(system_types, argtype) != hana::nothing,
                // The argument type is a stored system type:
                [&](auto _) { return std::get<std::reference_wrapper<ArgType>>(_systems); },
                [&](auto _) { static_assert(_(false), "System argument types are invalid"); }
              );
            }
          );
        });

        // hana::unpack(args, system);
        return hana::eval_if(
          std::is_invocable_v<ct::return_type_t<decltype(system)>>,
          [&](auto _) { return hana::unpack(args, _(system)); },
          [&](auto _) { hana::unpack(args, _(system)); return _([] {}); }
        );
      });
    }
  private:
    // System types in decayed form (removes cv-qualifiers and reference).
    static constexpr auto system_types = hana::transform(hana::tuple_t<TSystems...>, hana::traits::decay);
    // Set of component types used by any system in decayed form (removes cv-qualifiers and reference).
    static constexpr auto component_types = hana::difference(
      hana::to_set(hana::transform(
        hana::flatten(hana::make_tuple(to_hana_tuple_t<ct::args_t<TSystems>>...)),
        hana::traits::decay
      )),
      system_types
    );

    // The type of Storage used, determined by applying the associated component types as TStorage<...> template-parameters.
    using Storage = typename decltype(hana::unpack(component_types, hana::template_<TStorage>))::type;

    // Tuple to store references to the systems. (std::tuple instead of hana::tuple for std::get<> via type).
    std::tuple<std::reference_wrapper<TSystems>...> _systems;
    // Component Storage.
    Storage _storage;
  };

};

}