#include <iostream>

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <chrono>

#include "ecs/scaffold/ecs.hpp"

#include "ecs/storage/tuple_of_vectors.hpp"
#include "ecs/storage/heap.hpp"

#include "ecs/runtime/sequential.hpp"

#include "../util/frametime_system.hpp"

#if defined TUPLE_OF_VECTORS
using ECS = ecs::EntityComponentSystem<
  ecs::storage::TupleOfVectors,
  ecs::runtime::Sequential
>;
#elif defined HEAP
using ECS = ecs::EntityComponentSystem<
  ecs::storage::CustomHeap
    #ifdef HEAP_SMART
    ::WithSmartPointers
    #endif
    #ifdef HEAP_SET
      ::WithEntitySet
    #endif
    ::Storage
  ,
  ecs::runtime::Sequential
>;
#else
static_assert(false, "No storage strategy set.");
#endif

struct Transform {
  glm::vec2 pos;
};

struct Age {
  float seconds_live;
};

class MoveRightSystem {
public:
  void operator()(const benchmark::FrametimeSystem<1000>& frametime_system, ECS::Entity entity, float delta_time, Transform& transform, const Age& age) const {
    transform.pos = glm::vec2{
      320 - glm::sin(age.seconds_live * 2 - frametime_system.get_seconds_total()) * age.seconds_live * 45 / (1.0f + frametime_system.get_seconds_total() / 5),
      240 + glm::cos(age.seconds_live * 2 - frametime_system.get_seconds_total()) * age.seconds_live * 45 / (1.0f + frametime_system.get_seconds_total() / 5)
    };
  }
};

class RenderSystem {
public:
  RenderSystem(SDL_Surface* surface) : _surface(surface) {}

  void operator()(ECS::Entity entity, const Transform& transform, const Age& age) {
    SDL_Rect rect{int(transform.pos.x - 1), int(transform.pos.y - 1), 2, 2};
    SDL_FillRect(_surface, &rect, SDL_MapRGB(_surface->format,
      (std::sin(age.seconds_live / 20.0) + 1.0) * 0.5 * 0xff,
      (std::cos(age.seconds_live / 30.0) + 1.0) * 0.5 * 0xff,
      (std::sin(age.seconds_live / 30.0) + 1.0) * 0.5 * 0xff
    ));
  }
private:
  SDL_Surface* _surface;
};

class SpawnSystem {
public:
  SpawnSystem() {}

  auto operator()(double delta_time) {
    // bool spawn = false;
    // if (++step == 100) {
    //   spawn = true;
    //   step = 0;
    // }
    return [](const auto& manager) {
      for (auto i{0u}; i < 32; ++i)
        manager.new_entity(Transform{}, Age{0});
    };
  }
private:
  // uint32_t step = 0;
};

class AgeSystem {
public:
  auto operator()(Age& age, float delta_time) {
    age.seconds_live += delta_time;
  }
};

int main() {
  SDL_Window* window = NULL;
  SDL_Surface* screen_surface = nullptr;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow("ECS Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
  if(!window) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  screen_surface = SDL_GetWindowSurface(window);

  bool running = true;

  MoveRightSystem move_sys;
  ECS::Runtime tick(
    std::move(move_sys),
    SpawnSystem{},
    AgeSystem{},
    RenderSystem(screen_surface),
    benchmark::FrametimeSystem<1000>([&running]() { running = false; })
  );

  SDL_Event event;
  event.type = 0;
  while (running && event.type != SDL_QUIT) {
    SDL_FillRect(screen_surface, nullptr, SDL_MapRGB(screen_surface->format, 0x00, 0x00, 0x00));

    tick();

    SDL_UpdateWindowSurface(window);
    SDL_PollEvent(&event);
  }

  SDL_DestroyWindow( window );
  SDL_Quit();

  return 0;
}
