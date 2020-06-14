#include <iostream>

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include "ecs.hpp"
#include "storage/tuple_of_vectors.hpp"
#include "runtime/sequential.hpp"

using ECS = ecs::EntityComponentSystem<ecs::storage::TupleOfVectors, ecs::runtime::Sequential>;

struct Transform {
  glm::vec3 pos;
};

class MoveRightSystem {
public:
  void operator()(const ECS::Entity& entity, Transform& transform) {
    transform.pos =  glm::mod(transform.pos + glm::vec3{ 0.1 + entity * 0.1, glm::cos(entity) * 0.2, 0 }, glm::vec3{640, 480, 0});
  }
};

SDL_Surface* screen_surface = nullptr;

class RenderSystem {
public:
  void operator()(const Transform& transform) {
    SDL_Rect rect{int(transform.pos.x - 2), int(transform.pos.y - 2), 4, 4};
    SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0xff, 0x00, 0xff));
  }
};

int main() {
  SDL_Window* window = NULL;
  if (SDL_Init( SDL_INIT_VIDEO ) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN );
  if(!window) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  screen_surface = SDL_GetWindowSurface(window);

  ECS::Runtime<
    MoveRightSystem,
    RenderSystem
  > tick;


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