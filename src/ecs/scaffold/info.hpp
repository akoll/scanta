#pragma once

#include <boost/hana.hpp>
#include "ecs/util/callable_traits.hpp"

namespace hana = boost::hana;

namespace ecs {

template<typename TEntity, typename... TSystems>
struct Info {
  /// The system types in decayed form (without qualifiers).
  static constexpr auto systems = hana::transform(hana::tuple_t<TSystems...>, hana::traits::decay);

  /// Set of component types used by any system in decayed form (removes cv-qualifiers and reference).
  ///
  /// System-types and special types are discarded.
  static constexpr auto components = hana::difference(
    hana::to_set(hana::transform(
      hana::flatten(
        hana::make_tuple(to_hana_tuple_t<ct::args_t<TSystems>>...)
      ),
      hana::traits::decay
    )),
    hana::union_(
      hana::to_set(systems),
      hana::to_set(hana::tuple_t<
        TEntity,
        double, float // delta_time
      >)
    )
  );

  /// The parameter types required by a system.
  template<typename TSystem>
  static constexpr auto argtypes = hana::transform(argtypes_of<TSystem>, hana::traits::decay);

  /// The system parameter types that are also stored component types.
  template<typename TSystem>
  static constexpr auto component_argtypes = hana::intersection(hana::to_set(argtypes<TSystem>), components);
};

}
