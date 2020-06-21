#include <iostream>

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <chrono>

#include "ecs/ecs.hpp"
#include "ecs/storage/tuple_of_vectors.hpp"
#include "ecs/runtime/sequential.hpp"

using ECS = ecs::EntityComponentSystem<ecs::storage::TupleOfVectors, ecs::runtime::Sequential>;

struct Transform {
  glm::vec3 pos;
};

class MoveRightSystem {
public:
  void operator()(ECS::Entity entity, float delta_time, Transform& transform) {
    transform.pos =  glm::mod(
      glm::vec3{
        320 + glm::sin(transform.pos.z + entity * 0.1) * 100,
        240 + glm::cos(transform.pos.z + entity * (6.282 / 42)) * 100,
        transform.pos.z - 0.001 * delta_time},
      glm::vec3{640, 480, 6.283}
    );
  }
};

class RenderSystem {
public:
  RenderSystem(SDL_Surface* surface) : _surface(surface) {}

  void operator()(ECS::Entity entity, const Transform& transform) {
    SDL_Rect rect{int(transform.pos.x - 2), int(transform.pos.y - 2), 4, 4};
    SDL_FillRect(_surface, &rect, SDL_MapRGB(_surface->format,
      (std::sin(transform.pos.z) + 1.0) * 0.5 * 0xff,
      (std::cos(entity / 30.0) + 1.0) * 0.5 * 0x80,
      (std::sin(entity / 30.0) + 1.0) * 0.5 * 0xff
    ));
  }
private:
  SDL_Surface* _surface;
};

class FrametimeSystem {
public:
  FrametimeSystem() {
    _last = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
  }

  void operator()(double delta_time) {
    long long now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    if (now - _last >= 500) {
      std::cout << delta_time << "ms" << std::endl;
      _last = now;
    }
  }
private:
  long long _last;
};


int main() {
  SDL_Window* window = NULL;
  SDL_Surface* screen_surface = nullptr;

  if (SDL_Init( SDL_INIT_VIDEO ) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow( "ECS Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN );
  if(!window) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  screen_surface = SDL_GetWindowSurface(window);

  MoveRightSystem move_sys;
  ECS::Runtime tick(
    RenderSystem(screen_surface),
    move_sys,
    FrametimeSystem{}
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