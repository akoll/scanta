#pragma once

#include <stddef.h>
#include <concepts>

template<template<typename...> typename TStorage>
concept CStorage = requires(TStorage<size_t> storage) {
   { storage.template get_component<size_t>(0) } -> std::convertible_to<size_t&>;
} && !std::is_empty_v<typename TStorage<>::Entity>;