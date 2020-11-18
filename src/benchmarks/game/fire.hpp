#pragma once

#include "physics.hpp"

struct Flammable {
  bool on_fire = false;
};

class CombustionSystem {
public:
  auto operator()(const Flammable& flammable, const Collider& collider) const {
    return [&](const auto& manager) {
      for (auto entity : collider.collisions) {
        if (manager.template has_component<Flammable>(entity)) {
          manager.defer([&, entity](const auto& manager) {
            if (flammable.on_fire)
              manager.template get_component<Flammable>(entity).on_fire = true;
          });
        }
      }
    };
  }
};

struct Water {};

class WaterSystem {
public:
  auto operator()(const Water& flammable, const Collider& collider) const {
    return [&](const auto& manager) {
      for (auto entity : collider.collisions) {
        if (manager.template has_component<Flammable>(entity)) {
          manager.defer([&, entity](const auto& manager) {
            manager.template get_component<Flammable>(entity).on_fire = false;
          });
        }
      }
    };
  }
};
