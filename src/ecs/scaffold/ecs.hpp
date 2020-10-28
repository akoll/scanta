#pragma once

#include <utility>
#include <functional>

#include "storage.hpp"

/// Namespace containing the ecs library.
namespace ecs {

/// TODO: Document
///
/// @tparam TStorage The storage option to be used.
/// @tparam TRuntime The runtime option to be used.
template<template<typename...> typename TStorage, template<template<typename...> typename, typename...> typename TRuntime>
// TODO: Add CRuntime constraint.
requires CStorage<TStorage>
class EntityComponentSystem {
public:
  /// The Entity handle type to be used in service definitions.
  ///
  /// The Entity handle type of TStorage needs to be known here for services to be definable.
  /// Thus, it MAY NOT depend on the stored component types.
  using Entity = typename TStorage</* nothing */>::Entity;

  /// Combination of runtime and storage options for instantiating a callable logic pipeline using the specified storage option.
  ///
  /// This is effectively a type alias to the runtime option with template parameters partially set.
  /// (Unfortunately) this needs to be a class because template deduction guides are not implemented for alias templates (yet).
  template<typename... TSystems>
  class Scene : public TRuntime<TStorage, TSystems...> {
  public:
    // This constructor effectively acts as a template deduction guide
    // (which unfortunately aren't supported by gcc yet (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79501))
    // to infer the system types from the constructor parameters.
    /// This is not supported by clang and will not be necessary anymore, once gcc supports deduction guides.
    Scene(TSystems&&... systems) :
      TRuntime<TStorage, TSystems...>(std::forward<decltype(systems)>(systems)...)
    {}
  };

  /// Template deduction guide for inferring the system types from the constructor call.
  ///
  /// NOTE: While this is valid (see http://eel.is/c++draft/temp.deduct.guide#3.sentence-4)
  /// it is not currently supported by gcc (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79501).
  /// However, clang supports it (see https://bugs.llvm.org/show_bug.cgi?id=34520).
  ///
  /// As soon as gcc rolls out support, this deduction guide can be used, deprecating the runtime constructor.
  // template<typename... TSystems>
  // Runtime(TSystems&&...) -> Runtime<TSystems...>;
};

/// Namespace containing all runtime options which are shipped by default.
namespace runtime {}

/// Namespace containing all storage options which are shipped by default.
namespace storage {}

}
