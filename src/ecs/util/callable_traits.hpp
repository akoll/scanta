#pragma once

#include <boost/callable_traits.hpp>

#include "to_hana_tuple_t.hpp"

namespace ct = boost::callable_traits;

// Get the argument types of some callable as a hana::tuple_t.
template<typename T>
static constexpr auto argtypes_of = to_hana_tuple_t<ct::args_t<T>>;