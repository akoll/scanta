/// @file
/// @brief Utility to find the index of a type within a list of types.

#pragma once

#include <boost/hana/tuple.hpp>
namespace hana = boost::hana;
using namespace hana::literals;

/// Internal namespace only used in this header.
namespace internal {

/// Determines the index of the first instance of a type in a variadic list of types at compile-time.
///
/// Compilation will fail if the type is not found.
/// @tparam T The type to be searched.
/// @tparam Ts The list of types to be searched in.
template<typename T, typename... Ts>
consteval auto find_component_index() {
  // Return the first instance of the predicate being satisfied.
  auto index = hana::find_if(
    // Create an integer range from 0 to the amount of types searched (exclusively).
    hana::make_range(0_c, hana::size_c<sizeof...(Ts)>),
    // Determine wether the type at the current integer from the range is the searched type.
    [](auto index) { return hana::tuple_t<Ts...>[index] == hana::type_c<T>; }
  );
  // Fail if the type is not found.
  static_assert(index != hana::nothing, "The type to be indexed is not in the list.");
  return index.value();
}

}

/// The index of the first instance of a type in a variadic list of types.
///
/// Compilation will fail if the type is not in the list.
/// @tparam T The type to be searched.
/// @tparam Ts The list of types to be searched in.
template<typename T, typename... Ts>
constexpr size_t type_index = internal::find_component_index<T, Ts...>();

/// Whether or not a specific type is contained in a variadic list of types.
///
/// @tparam T The type in question.
/// @tparam Ts The list of types to be searched in.
template<typename T, typename... Ts>
constexpr bool types_contain = hana::contains(hana::tuple_t<Ts...>, hana::type_c<T>);
