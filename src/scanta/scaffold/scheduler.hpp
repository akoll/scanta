#pragma once

#include "info.hpp"
#include "storage.hpp"

namespace scanta {

template<template<typename...> typename TStorage, typename... TSystems>
requires CStorage<TStorage>
class Scheduler {
public:
  using Entity = typename TStorage<>::Entity;
protected:
  using Info = scanta::Info<Entity, TSystems...>;

  // The type of Storage used, determined by applying the associated component types as TStorage<...> template-parameters.
  using Storage = typename decltype(hana::unpack(Info::components, hana::template_<TStorage>))::type;

  /// The runtime manager to be passed into system executions.
  ///
  /// Systems may need to be able to execute certain scheduler operations
  /// and access the underlying storage. Therefor, a _manager_ is provided
  /// to the system execution, on which such operations can be done.
  ///
  /// @tparam TScheduler The scheduler type to be managed.
  template<typename TScheduler>
  class RuntimeManager {
  public:
    /// Constructs a manager for some scheduler and storage.
    ///
    /// @param scheduler The scheduler to be managed.
    /// @param storage The storage to be managed.
    RuntimeManager(TScheduler& scheduler, Storage& storage) :
      _scheduler(scheduler),
      _storage(storage)
    {}

    // Immediately executed functions:

    /// Returns the size of the underlying entity storage.
    ///
    /// Note that depending on the storage implementation, this might also be
    /// just an upper bound when called non-deferred.
    size_t get_entity_count() const {
      return _storage.get_size();
    }

    /// Defers an operation by queuing it with the scheduler.
    ///
    /// @param operation The operation to be deferred.
    void defer(auto&& operation) const {
      _scheduler.defer(operation);
    }

    // Deferred functions:

    /// Creates a new entity in the scene.
    ///
    /// When called, the entity is not created immediately,
    /// but merely queued as a deferred operation.
    ///
    /// @param components The set of components to be initially associated with the new entity.
    void new_entity(auto&&... components) const {
      defer([&](const auto& manager) {
        manager.new_entity(std::forward<decltype(components)>(components)...);
      });
    }

    /// Removes an entity from the scene.
    ///
    /// When called, the entity is not removed immediately,
    /// but merely queued as a deferred operation.
    ///
    /// @param entity The entity to be removed.
    void remove_entity(Entity entity) const {
      defer([entity](const auto& manager) {
        manager.remove_entity(entity);
      });
    }
  protected:
    /// The scheduler managed by this manager.
    TScheduler& _scheduler;
    /// The storage managed by this manager.
    Storage& _storage;
  };

  /// The deferred manager to be passed into deferred operation executions.
  ///
  /// Deferred operations may need to be able to execute certain scheduler operations
  /// and access the underlying storage. Therefor, a _manager_ is provided
  /// to the deferred call, on which such operations can be done.
  ///
  /// This manager's methods are a superset of the runtime manager's.
  ///
  /// @tparam TScheduler The scheduler type to be managed.
  template<typename TScheduler>
  class DeferredManager : public RuntimeManager<TScheduler> {
  public:
    /// Constructs a manager for some sequential scheduler.
    ///
    /// @param scheduler The scheduler to be managed.
    /// @param storage The storage to be managed.
    DeferredManager(TScheduler& scheduler, Storage& storage) : RuntimeManager<TScheduler>(scheduler, storage) {}

    // Include scheduler manager functionality.
    using RuntimeManager<TScheduler>::get_entity_count;

    /// Creates a new entity in the scene.
    ///
    /// @param components The set of components to be initially associated with the new entity.
    inline void new_entity(auto&&... components) const {
      _storage.new_entity(std::forward<decltype(components)>(components)...);
    }

    /// Removes an entity from the scene.
    ///
    /// @param entity The entity to be removed.
    inline void remove_entity(Entity entity) const {
      _storage.remove_entity(entity);
    }
  protected:
    using RuntimeManager<TScheduler>::_scheduler;
    using RuntimeManager<TScheduler>::_storage;
  };
};

}
