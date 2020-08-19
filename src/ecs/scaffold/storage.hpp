#pragma once

#include <stddef.h>
#include <concepts>

template<template<typename...> typename TStorage>
concept CStorage =
  !std::is_empty_v<typename TStorage<>::Entity> &&
  requires(TStorage<size_t> storage) {
   // TODO: are all entity IDs default-constructible?
   { storage.template get_component<size_t>(typename TStorage<>::Entity{0}) } -> std::convertible_to<size_t&>;
  }
;