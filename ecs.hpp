#pragma once

#include <utility>
#include <functional>

#include "storage/storage.hpp"

namespace ecs {

// Storage{TStorage}
template<template<typename...> typename TStorage, template<template<typename...> typename, typename...> typename TRuntime>
requires Storage<TStorage>
class EntityComponentSystem {
public:
  // The Entity handle type of TStorage needs to be known here for services to be definable.
  // Thus, it MAY NOT depend on the stored component types.
  using Entity = typename TStorage</* nothing */>::Entity;

  // The Runtime template for instantiating a callable logic pipeline.
  template<typename... TSystems>
  using Runtime = TRuntime<TStorage, TSystems...>;
};

}