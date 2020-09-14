#pragma once

#include <boost/hana.hpp>
#include "ecs/util/callable_traits.hpp"

namespace hana = boost::hana;
namespace ct = boost::callable_traits;

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

  /// The parameter types required by a system in decayed form.
  template<typename TSystem>
  static constexpr auto argtypes = hana::transform(argtypes_of<TSystem>, hana::traits::decay);

  /// The decayed system parameter types that are also stored component types.
  template<typename TSystem>
  static constexpr auto component_argtypes = hana::intersection(hana::to_set(argtypes<TSystem>), components);

  /// The non-decayed system parameter types that are also system types.
  template<typename TSystem>
  static constexpr auto system_argtypes = hana::filter(argtypes_of<TSystem>, [](auto argtype) consteval {
    return hana::find(systems, hana::traits::decay(argtype)) != hana::nothing;
  });

  /// Whether a system allows for inner parallelism or not.
  ///
  /// A system is considered parallelizable if it does not alter state of itself or any other system.
  /// This is the case when it is marked `const` and does not have any non-const system references as parameters.
  /// This also means that intentionally _not_ marking systems as const allows for the parallelism to
  /// be disabled manually in cases where side-effects shall be considered outside the systems (e.g., I/O).
  template<typename TSystem>
  static constexpr bool parallelizable =
    // A system shall not be parallelized if it is not marked const.
    ct::is_const_member_v<TSystem>
    // A system shall not be parallelized if non-const system parameters exist.
    && (hana::find_if(
      system_argtypes<TSystem>,
      [](auto argtype) consteval {
        return hana::bool_c<
          !std::is_const_v<std::remove_reference_t<typename decltype(argtype)::type>>
        >;
      }
    ) == hana::nothing);
  ;
};

}
