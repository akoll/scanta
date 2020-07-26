#pragma once

#include <boost/hana.hpp>
#include "util/callable_traits.hpp"
#include "manager.hpp"

// System method names.
#define SYSTEM_START_METHOD start // Called once at runtime creation.
#define SYSTEM_UPDATE_METHOD update // Called each frame on each (matching) entity.

namespace hana = boost::hana;

namespace ecs {

namespace internal {
  template<typename TSystem>
  concept HasStart = requires(TSystem system) {
    { system.start() };
  };
}

template<typename TSystem>
struct SystemInfo {
  static constexpr bool has_start = internal::HasStart<TSystem>;
  static constexpr auto update_arguments = to_hana_tuple_t<ct::args_t<decltype(&std::decay_t<TSystem>::SYSTEM_UPDATE_METHOD)>>;
};

template<typename TEntity, typename... TSystems>
struct Info {
  static constexpr auto systems = hana::transform(hana::tuple_t<TSystems...>, hana::traits::decay);

  // Set of component types used by any system in decayed form (removes cv-qualifiers and reference).
  // System-types and special types are discarded.
  static constexpr auto components = hana::difference(
    hana::to_set(hana::transform(
      hana::flatten(
        hana::make_tuple(SystemInfo<TSystems>::update_arguments...)
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