#include <iostream>
#include <SDL2/SDL.h>

#include "util/benchmark.hpp"
#include "game/game.hpp"

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  Game<ECS, benchmark::Scene> game;
  game.run();

  SDL_Quit();
  return 0;
}
