#pragma once

#include <utility>
#include <functional>

#include "storage.hpp"

/// Namespace containing the ecs library.
namespace ecs {

/// TODO: Document
///
/// @tparam TStorage The storage option to be used.
/// @tparam TScheduler The scheduler option to be used.
template<template<typename...> typename TStorage, template<template<typename...> typename, typename...> typename TScheduler>
// TODO: Add CScheduler constraint.
requires CStorage<TStorage>
class EntityComponentSystem {
public:
  /// The Entity handle type to be used in service definitions.
  ///
  /// The Entity handle type of TStorage needs to be known here for services to be definable.
  /// Thus, it MAY NOT depend on the stored component types.
  using Entity = typename TStorage</* nothing */>::Entity;

  /// Combination of scheduler and storage options for instantiating a callable logic pipeline using the specified storage option.
  ///
  /// This is effectively a type alias to the scheduler option with template parameters partially set.
  /// (Unfortunately) this needs to be a class because template deduction guides are not implemented for alias templates (yet).
  template<typename... TSystems>
  class Scene : public TScheduler<TStorage, TSystems...> {
  public:
    // This constructor effectively acts as a template deduction guide
    // (which unfortunately aren't supported by gcc yet (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79501))
    // to infer the system types from the constructor parameters.
    /// This is not supported by clang and will not be necessary anymore, once gcc supports deduction guides.
    Scene(TSystems&&... systems) :
      TScheduler<TStorage, TSystems...>(std::forward<decltype(systems)>(systems)...)
    {}
  };

  /// Template deduction guide for inferring the system types from the constructor call.
  ///
  /// NOTE: While this is valid (see http://eel.is/c++draft/temp.deduct.guide#3.sentence-4)
  /// it is not currently supported by gcc (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79501).
  /// However, clang supports it (see https://bugs.llvm.org/show_bug.cgi?id=34520).
  ///
  /// As soon as gcc rolls out support, this deduction guide can be used, deprecating the scheduler constructor.
  // template<typename... TSystems>
  // Scheduler(TSystems&&...) -> Scheduler<TSystems...>;
};

/// Namespace containing all scheduler options which are shipped by default.
namespace scheduler {}

/// Namespace containing all storage options which are shipped by default.
namespace storage {}

}
