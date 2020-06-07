#pragma once

#include "storage/storage.hpp"

#include <utility>
#include <functional>

namespace ecs {

// Storage{TStorage}
template<template<typename...> typename TStorage, template<template<typename...> typename, typename...> typename TRuntime>
requires Storage<TStorage>
class EntityComponentSystem {
public:
  // The Entity handle type of TStorage needs to be known here for services to be definable.
  // Thus, it MAY NOT depend on the stored component types.
  using Entity = typename TStorage</* nothing */>::Entity;

  template<typename... TSystems>
  static auto make_runtime(TSystems&&... systems) { return TRuntime<TStorage, TSystems...>(std::forward<TSystems>(systems)...); }

  template<typename... TSystems>
  class Runtime {
  public:
    // template<typename... TSystems>
    constexpr Runtime(TSystems&&... systems) : _runtime(TRuntime<TStorage, TSystems...>(std::forward<TSystems>(systems)...)) {}
    constexpr auto operator()(auto... args) {
      return _runtime(std::forward(args)...);
    }
  private:
    TRuntime<TStorage, TSystems...> _runtime;
  };

  // template deduction guide to support copying in lvalue-references
  // NOTE: while this is valid (see http://eel.is/c++draft/temp.deduct.guide#3.sentence-4)
  // it is not currently supported by gcc (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79501)
  // template<typename... TSystems>
  // Runtime(TSystems&&...) -> Runtime<TSystems...>;

};

}