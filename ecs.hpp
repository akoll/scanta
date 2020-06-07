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

  // Template deduction guide to support lvalue-references in constructor.
  // NOTE: While this is valid (see http://eel.is/c++draft/temp.deduct.guide#3.sentence-4)
  // it is not currently supported by gcc (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79501).
  // However, clang supports it (see https://bugs.llvm.org/show_bug.cgi?id=34520).
  template<typename... TSystems>
  Runtime(TSystems&&...) -> Runtime<TSystems...>;

};

}