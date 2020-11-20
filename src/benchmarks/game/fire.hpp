#pragma once

#include "physics.hpp"
#include "player.hpp"

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

class FireDamageSystem {
public:
  auto operator()(ECS::Entity entity, const Flammable& flammable, Mortal& mortal, float delta_time) {
    return [&, entity, delta_time](const auto& manager) {
      if (flammable.on_fire) {
        mortal.hp -= delta_time;
        if (mortal.hp <= 0) manager.remove_entity(entity);
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
