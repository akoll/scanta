#pragma once

template<template<typename...> typename TStorage>
concept Storage = requires(TStorage<size_t> storage) {
  { storage.template getComponent<size_t>(0) } -> size_t&;
};