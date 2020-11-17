#include <iostream>
#include <SDL2/SDL.h>

#include <iostream>

#include <SDL2/SDL.h>
#include <string>
#include <chrono>

#include <boost/callable_traits.hpp>
namespace ct = boost::callable_traits;

#include "physics.hpp"
#include "input.hpp"
#include "player.hpp"
#include "render.hpp"

template<typename ECS, template<typename...> typename TScene = ECS::template Scene>
class Game {
public:

  Game() :
    _window(SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN)),
    _renderer(SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED)),
    _scene(
      InputSystem{},
      ControllerSystem{},
      MovementSystem{},
      CollisionSystem{},
      RectangleRenderSystem(_renderer),
      SpriteRenderSystem(_renderer)
    )
  {
    if(!_window) {
      std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
      exit(1);
    }
    if(!_renderer) {
      std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
      exit(1);
    }

    constexpr auto file = "../game/player.png";
    SDL_Surface* surface = IMG_Load(file);
    if (!surface) std::cout << "Failed to load '" << file << "'." << std::endl;
    _player_texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_FreeSurface(surface);

    _scene->manager.new_entity(
      Playable{},
      Transform{{320, 240}},
      RigidBody{},
      Sprite{_player_texture},
      Collider{{-16, -16, 16, 16}}
    );

    _scene->manager.new_entity(
      Transform{{0, 0}},
      Rectangle{{0, 0, 100, 100}, {0xff, 0x00, 0xff}},
      Collider{{0, 0, 100, 100}}
    );
  }

  ~Game() {
    SDL_DestroyTexture(_player_texture);

    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
  }

  void run() {
    bool running = true;
    while (running && !_scene->template get_system<InputSystem>().is_quit()) {
      SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
      SDL_RenderClear(_renderer);

      if constexpr (requires { { _scene.update() } -> std::convertible_to<bool>; })
        running = _scene.update();
      else
        _scene.update();

      SDL_RenderPresent(_renderer);
    }
  }

private:
  SDL_Window* _window = nullptr;
  SDL_Renderer* _renderer = nullptr;

  SDL_Texture* _player_texture = nullptr;

  TScene<
    InputSystem,
    ControllerSystem,
    MovementSystem,
    CollisionSystem,
    RectangleRenderSystem,
    SpriteRenderSystem
  > _scene;
};
