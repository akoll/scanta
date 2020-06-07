#pragma once

#include <stddef.h>
#include <type_traits>

template<template<typename...> typename TStorage>
concept Storage = /* requires(TStorage<size_t> storage) {
   { storage.template get_component<size_t>(0) } -> size_t&;
} && */ !std::is_empty_v<typename TStorage<>::Entity>;