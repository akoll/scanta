#pragma once

#include "to_hana_tuple_t.hpp"
#include <boost/callable_traits.hpp>
namespace ct = boost::callable_traits;

// Get the argument types of some callable as a hana::tuple_t.
template<typename T>
static constexpr auto get_argtypes(const T& _) {
  return to_hana_tuple_t<ct::args_t<T>>;
}