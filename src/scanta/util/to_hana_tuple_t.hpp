/// @file
/// @brief Utility to convert a variadic template of the form T<Ts...> into a hana::tuple_t<Ts...>, discarding T.

#pragma once

#include <boost/hana/tuple.hpp>
namespace hana = boost::hana;

/// Internal namespace only used in this header.
namespace internal {

/// Base declaration.
template<typename...>
struct ToHanaTuple;

/// Partial specialization on variadic templates.
template<template<typename...> typename T, typename... Ts>
struct ToHanaTuple<T<Ts...>> {
  static constexpr auto tuple = hana::tuple_t<Ts...>;
};

}

/// The converted `boost::hana::tuple_t` from some variadic template.
template<typename T>
constexpr auto to_hana_tuple_t = internal::ToHanaTuple<T>::tuple;