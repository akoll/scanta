#include <iostream>

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <chrono>

#include "ecs/ecs.hpp"
#include "ecs/manager.hpp"

#include "ecs/storage/heap_smart.hpp"
#include "ecs/runtime/sequential.hpp"

using ECS = ecs::EntityComponentSystem<ecs::storage::SmartHeap, ecs::runtime::Sequential>;

struct Transform {
  glm::vec2 pos;
};

struct Age {
  float seconds_live;
};

struct Lifetime {
  float seconds_left;
};


class MoveRightSystem {
public:
  void operator()(ECS::Entity entity, float delta_time, Transform& transform, const Age& age) {
    transform.pos.x -= delta_time * (200 + glm::sin(age.seconds_live * 10) * 100);
    transform.pos.y += delta_time * glm::sin(age.seconds_live * 3) * 50;
  }
};

class RenderSystem {
public:
  RenderSystem(SDL_Surface* surface) : _surface(surface) {}

  void operator()(ECS::Entity entity, const Transform& transform, const Age& age, const Lifetime& lifetime) {
    SDL_Rect rect{int(transform.pos.x - 4), int(transform.pos.y - 4), 8, 8};
    Uint8 col = std::max(lifetime.seconds_left / 3 * 0xff, 0.0f);
    SDL_FillRect(_surface, &rect, SDL_MapRGB(_surface->format,
      (std::sin(age.seconds_live) + 1.0) * 0.5 * col,
      (std::cos(age.seconds_live / 30.0) + 1.0) * 0.5 * col,
      (std::sin(age.seconds_live / 30.0) + 1.0) * 0.5 * col
    ));
  }
private:
  SDL_Surface* _surface;
};

class FrametimeSystem {
public:
  FrametimeSystem() : _last(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()) {}

  void operator()(const ecs::RuntimeManager& manager, double delta_time) {
    long long now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    if (now - _last >= 500) {
      std::cout << manager.get_entity_count() << "#, " << delta_time * 1000 << "ms" << std::endl;
      _last = now;
    }
  }
private:
  long long _last;
};

class SpawnSystem {
public:
  SpawnSystem() : _last(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()) {}

  auto operator()(const ecs::RuntimeManager& manager, double delta_time) {
    bool spawn = false;
    long long now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    if (now - _last >= 10) {
      _last = now;
      spawn = true;
    }
    return [spawn, now](const auto& manager) {
      if (spawn) manager.new_entity(Transform{{640, 240 - glm::sin(now * 0.002) * 170}}, Lifetime{3}, Age{0});
    };
  }
private:
  long long _last;
};

class LifetimeSystem {
public:
  auto operator()(ECS::Entity entity, Age& age, Lifetime& lifetime, float delta_time) {
    lifetime.seconds_left -= delta_time;
    age.seconds_live += delta_time;
    bool kill = lifetime.seconds_left <= 0;
    return [entity, kill](const auto& manager) {
      if (kill) manager.remove_entity(entity);
    };
  }
};

int main() {
  SDL_Window* window = NULL;
  SDL_Surface* screen_surface = nullptr;

  if (SDL_Init( SDL_INIT_VIDEO ) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow("ECS Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
  if(!window) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  screen_surface = SDL_GetWindowSurface(window);

  MoveRightSystem move_sys;
  ECS::Runtime tick(
    std::move(move_sys),
    FrametimeSystem{},
    SpawnSystem{},
    LifetimeSystem{},
    RenderSystem(screen_surface)
  );


  SDL_Event event;
  while (event.type != SDL_QUIT) {
    SDL_FillRect(screen_surface, nullptr, SDL_MapRGB(screen_surface->format, 0x00, 0x00, 0x00));

    tick();

    SDL_UpdateWindowSurface(window);
    SDL_PollEvent(&event);
  }

  SDL_DestroyWindow( window );
  SDL_Quit();


  return 0;
}