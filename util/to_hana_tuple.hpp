/**
 * Utility to convert a variadic template of the form T<Ts...>
 * into a hana::tuple_t<Ts...>, discarding T.
**/

#pragma once

#include <boost/hana/tuple.hpp>
namespace hana = boost::hana;

// internal namespace only used in this header
namespace internal {

// Forward declaration
template<typename...>
struct ToHanaTuple;

// Partial specialization on variadic templates
template<template<typename...> typename T, typename... Ts>
struct ToHanaTuple<T<Ts...>> {
  static constexpr auto tuple = hana::tuple_t<Ts...>;
};

}

// Expose tuple value
template<typename T>
constexpr auto to_hana_tuple = internal::ToHanaTuple<T>::tuple;