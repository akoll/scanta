#pragma once

#include <boost/hana.hpp>
#include "util/callable_traits.hpp"
#include "manager.hpp"

namespace hana = boost::hana;

namespace ecs {

template<typename TEntity, typename... TSystems>
struct Info {
  static constexpr auto systems = hana::transform(hana::tuple_t<TSystems...>, hana::traits::decay);

  // Set of component types used by any system in decayed form (removes cv-qualifiers and reference).
  // System-types and special types are discarded.
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
        double, float, // delta_time
        ecs::RuntimeManager
      >)
    )
  );
};

}