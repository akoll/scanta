#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>

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
      int(rectangle.bounds.x),
      int(rectangle.bounds.y),
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
};

class SpriteRenderSystem {
public:
  SpriteRenderSystem(SDL_Renderer* renderer) : _renderer(renderer) {}

  void operator()(const Transform& transform, const Sprite& sprite, const RectangleRenderSystem&) {
    SDL_SetRenderDrawColor(_renderer,
      (std::sin(0 / 20.0) + 1.0) * 0.5 * 0xff,
      (std::cos(0 / 30.0) + 1.0) * 0.5 * 0xff,
      (std::sin(0 / 30.0) + 1.0) * 0.5 * 0xff,
      0xff
    );
    SDL_Rect rect{int(transform.pos.x - 16), int(transform.pos.y - 16), 32, 32};

    SDL_RenderCopy(_renderer, sprite.texture, nullptr, &rect);
  }
private:
  SDL_Renderer* _renderer;
};
