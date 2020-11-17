#pragma once

#include <vector>
#include <unordered_set>
#include <glm/glm.hpp>

struct Transform {
  glm::vec2 pos{0, 0};
};

struct RigidBody {
  glm::vec2 velocity{0, 0};
};

class MovementSystem {
public:
  void operator()(Transform& transform, const RigidBody& body, float delta_time) const {
    transform.pos += body.velocity * delta_time;
  }
};

struct Collider {
  glm::vec4 bounds;
};

class CollisionSystem {
public:
  // NOTE: This is very inefficient.
  auto operator()(ECS::Entity entity, const Transform& transform, const Collider& collider) {
    // Clear collision sets.
    return [&,entity](const auto& manager) {
      for (auto group : _collisions) {
        for (auto other : group) {
          auto [ other_entity, other_transform, other_collider ] = other;
          if (
            other_collider.get().bounds.z + other_transform.get().pos.x > collider.bounds.x + transform.pos.x
            && other_collider.get().bounds.x + other_transform.get().pos.x < collider.bounds.z + transform.pos.y
            && other_collider.get().bounds.w + other_transform.get().pos.y > collider.bounds.y + transform.pos.y
            && other_collider.get().bounds.y + other_transform.get().pos.y < collider.bounds.w + transform.pos.y
          ) {
            group.push_back({ entity, transform, collider });
            _handle_collision(manager, entity, other_entity);
            goto done;
          }
        }
      }
      _collisions.push_back({{ entity, transform, collider }});

      done:

      if (_fresh) {
        _fresh = false;
        manager.defer([this](const auto&) {
          _collisions.clear();
          _fresh = true;
        });
      }
    };
  }
private:
  bool _fresh = true;
  // TODO: Optimize with a quadtree.
  std::vector<std::vector<
    std::tuple<
      ECS::Entity,
      std::reference_wrapper<const Transform>,
      std::reference_wrapper<const Collider>
    >
  >> _collisions;

  void _handle_collision(const auto& manager, ECS::Entity a, ECS::Entity b) {
    std::cout << a << " and " << b << std::endl;
  }
};
