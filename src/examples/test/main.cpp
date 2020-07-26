#include <iostream>

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <chrono>

#include "ecs/ecs.hpp"
#include "ecs/manager.hpp"

#include "ecs/storage/heap.hpp"
#include "ecs/runtime/sequential.hpp"

using ECS = ecs::EntityComponentSystem<ecs::storage::Heap, ecs::runtime::Sequential>;

struct Transform {
  glm::vec3 pos;
};

struct Age {
  float millis_live;
};

struct Lifetime {
  float millis_left;
};


class MoveRightSystem {
public:
  void update(ECS::Entity entity, float delta_time, Transform& transform, const Age& age) {
    transform.pos =  glm::mod(
      glm::vec3{
        // 320 + glm::sin(transform.pos.z + entity * 0.01) * 250,
        320 + glm::sin(transform.pos.z + age.millis_live * (2.0 / 210)) * 170,
        240 + glm::cos(transform.pos.z + age.millis_live * (2.0 / 420)) * 170,
        transform.pos.z - 0.001 * delta_time},
      glm::vec3{640, 480, 6.283}
    );
  }
};

class RenderSystem {
public:
  RenderSystem(SDL_Surface* surface) : _surface(surface) {}

  void update(ECS::Entity entity, const Transform& transform, const Age& age, const Lifetime& lifetime) {
    SDL_Rect rect{int(transform.pos.x - 2), int(transform.pos.y - 2), 4, 4};
    Uint8 col = std::max(lifetime.millis_left / 500 * 0xff, 0.0f);
    SDL_FillRect(_surface, &rect, SDL_MapRGB(_surface->format,
      (std::sin(transform.pos.z) + 1.0) * 0.5 * col,
      (std::cos(age.millis_live / 30.0) + 1.0) * 0.5 * col,
      (std::sin(age.millis_live / 30.0) + 1.0) * 0.5 * col
    ));
  }
private:
  SDL_Surface* _surface;
};

class FrametimeSystem {
public:
  FrametimeSystem() : _last(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()) {}

  void update(const ecs::RuntimeManager& manager, double delta_time) {
    long long now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    if (now - _last >= 500) {
      std::cout << manager.get_entity_count() << "#, " << delta_time << "ms" << std::endl;
      _last = now;
    }
  }
private:
  long long _last;
};

class SpawnSystem {
public:
  SpawnSystem() : _last(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()) {}

  auto update(const ecs::RuntimeManager& manager, double delta_time) {
    bool spawn = false;
    long long now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    if (now - _last >= 5) {
      _last = now;
      spawn = true;
    }
    return [spawn]<ecs::DeferredManager Manager>(const Manager& manager) {
      if (spawn) manager.new_entity(Transform{}, Lifetime{500}, Age{0});
    };
  }
private:
  long long _last;
};

class LifetimeSystem {
public:
  auto update(ECS::Entity entity, Age& age, Lifetime& lifetime, float delta_time) {
    lifetime.millis_left -= delta_time;
    age.millis_live += delta_time;
    bool kill = lifetime.millis_left <= 0;
    return [entity, kill]<ecs::DeferredManager Manager>(const Manager& manager) {
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