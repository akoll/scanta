#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>

#include "fire.hpp"

struct Rectangle {
  glm::vec4 bounds;
  glm::vec<3, uint8_t> color;
};

class RectangleRenderSystem {
public:
  RectangleRenderSystem(SDL_Renderer* renderer) : _renderer(renderer) {}

  void operator()(const Transform& transform, const Rectangle& rectangle) {
    SDL_SetRenderDrawColor(_renderer,
      rectangle.color.r,
      rectangle.color.g,
      rectangle.color.b,
      0xff
    );

    SDL_Rect rect{
      int(transform.pos.x + rectangle.bounds.x),
      int(transform.pos.y + rectangle.bounds.y),
      int(rectangle.bounds.z - rectangle.bounds.x),
      int(rectangle.bounds.w - rectangle.bounds.y)
    };

    SDL_RenderFillRect(_renderer, &rect);
  }
private:
  SDL_Renderer* _renderer;
};

struct Sprite {
  SDL_Texture* texture = nullptr;
  glm::vec2 dimensions;
};

class SpriteRenderSystem {
public:
  SpriteRenderSystem(SDL_Renderer* renderer) : _renderer(renderer) {}

  void operator()(const Transform& transform, const Sprite& sprite, const RectangleRenderSystem&) {
    SDL_Rect rect{
      int(transform.pos.x - (sprite.dimensions.x / 2)),
      int(transform.pos.y - (sprite.dimensions.y / 2)),
      int(sprite.dimensions.x),
      int(sprite.dimensions.y)
    };

    SDL_RenderCopy(_renderer, sprite.texture, nullptr, &rect);
  }
private:
  SDL_Renderer* _renderer;
};

class FireRenderSystem {
public:
  FireRenderSystem(SDL_Renderer* renderer) : _renderer(renderer) {
    constexpr auto file = "../game/fire.png";
    SDL_Surface* surface = IMG_Load(file);
    if (!surface) std::cout << "Failed to load '" << file << "'." << std::endl;
    _texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_FreeSurface(surface);
  }

  void operator()(const Transform& transform, const Flammable& flammable, const SpriteRenderSystem&) {
    if (flammable.on_fire) {
      SDL_Rect rect{int(transform.pos.x - 24), int(transform.pos.y - 24), 48, 48};
      SDL_RenderCopy(_renderer, _texture, nullptr, &rect);
    }
  }
private:
  SDL_Renderer* _renderer;
  SDL_Texture* _texture;
};

class HitpointsRenderSystem {
public:
  HitpointsRenderSystem(SDL_Renderer* renderer) : _renderer(renderer) {}

  void operator()(const Transform& transform, const Mortal& mortal, const FireRenderSystem&) {
    auto prec = glm::max(mortal.hp / mortal.max_hp, 0.0f);

    SDL_SetRenderDrawColor(_renderer, 0xff, 0x00, 0x00, 0xff);
    SDL_Rect rect_bg{
      int(transform.pos.x - 10),
      int(transform.pos.y + 20),
      int(20),
      5
    };
    SDL_RenderFillRect(_renderer, &rect_bg);

    SDL_SetRenderDrawColor(_renderer, 0x00, 0xff, 0x00, 0xff);
    SDL_Rect rect_fg{
      int(transform.pos.x - 10),
      int(transform.pos.y + 20),
      int(prec * 20),
      5
    };
    SDL_RenderFillRect(_renderer, &rect_fg);
  }
private:
  SDL_Renderer* _renderer;
};
