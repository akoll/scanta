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
#include "fire.hpp"

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
      CombustionSystem{},
      WaterSystem{},
      FireDamageSystem{},
      CollisionSystem{},
      ColliderCleanupSystem{},
      RectangleRenderSystem(_renderer),
      SpriteRenderSystem(_renderer),
      FireRenderSystem(_renderer),
      HitpointsRenderSystem(_renderer)
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

    SDL_Surface* surface = IMG_Load("../game/player.png");
    if (!surface) std::cout << "Failed to load '../game/player.png'." << std::endl;
    _player_texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("../game/barrel.png");
    if (!surface) std::cout << "Failed to load '../game/barrel.png'." << std::endl;
    _barrel_texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_FreeSurface(surface);

    _scene->manager.new_entity(
      Playable{},
      Transform{{320, 240}},
      RigidBody{},
      Sprite{_player_texture, {32, 32}},
      Collider{{-16, -16, 16, 16}},
      Flammable{},
      Mortal{10}
    );

    _scene->manager.new_entity(
      Transform{{420, 300}},
      RigidBody{},
      Sprite{_player_texture, {32, 32}},
      Collider{{-16, -16, 16, 16}},
      Flammable{},
      Mortal{2}
    );

    _scene->manager.new_entity(
      Transform{{150, 150}},
      Sprite{_barrel_texture, {48, 64}},
      Collider{{-32, -32, 32, 32}},
      Flammable{true}
    );

    _scene->manager.new_entity(
      Transform{{230, 70}},
      Sprite{_barrel_texture, {48, 64}},
      Collider{{-32, -32, 32, 32}},
      Flammable{}
    );

    _scene->manager.new_entity(
      Transform{{430, 200}},
      Sprite{_barrel_texture, {48, 64}},
      Collider{{-32, -32, 32, 32}},
      Flammable{}
    );

    _scene->manager.new_entity(
      Transform{{325, 300}},
      Rectangle{{-32, -32, 32, 32}, {0x00, 0x00, 0xff}},
      Collider{{-32, -32, 32, 32}},
      Water{}
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
      SDL_SetRenderDrawColor(_renderer, 30, 110, 32, 0);
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
  SDL_Texture* _barrel_texture = nullptr;

  TScene<
    InputSystem,
    ControllerSystem,
    MovementSystem,
    CombustionSystem,
    WaterSystem,
    FireDamageSystem,
    CollisionSystem,
    ColliderCleanupSystem,
    RectangleRenderSystem,
    SpriteRenderSystem,
    FireRenderSystem,
    HitpointsRenderSystem
  > _scene;
};
