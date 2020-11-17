#pragma once

#include "input.hpp"
#include "physics.hpp"

struct Playable {
  float speed = 100.0f;
};

class ControllerSystem {
public:
  void operator()(const InputSystem& input, const Playable& player, RigidBody& body) const {
    glm::vec2 dir{0, 0};

    if (input.is_key_down<SDLK_w>()) dir += glm::vec2{0, -1};
    if (input.is_key_down<SDLK_a>()) dir += glm::vec2{-1, 0};
    if (input.is_key_down<SDLK_s>()) dir += glm::vec2{0, 1};
    if (input.is_key_down<SDLK_d>()) dir += glm::vec2{1, 0};

    if (dir != glm::vec2{0, 0}) dir = glm::normalize(dir);
    body.velocity = dir * player.speed;
  }
};
