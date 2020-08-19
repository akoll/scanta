#pragma once

#include "info.hpp"
#include "storage.hpp"

namespace ecs {

template<template<typename...> typename TStorage, typename... TSystems>
requires CStorage<TStorage>
class Runtime {
public:
  using Entity = typename TStorage<>::Entity;
protected:
  using Info = ecs::Info<Entity, TSystems...>;

  // The type of Storage used, determined by applying the associated component types as TStorage<...> template-parameters.
  using Storage = typename decltype(hana::unpack(Info::components, hana::template_<TStorage>))::type;
  Storage _storage;
};

}