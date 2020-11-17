#pragma once

#include <iostream>
#include <SDL2/SDL.h>

class InputSystem {
public:
  void operator()() {

    SDL_Event event;
    event.type = 0;
    SDL_PollEvent(&event);

    switch (event.type) {
      case SDL_QUIT:
        _quit = true;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym < key_count)
          _keys[event.key.keysym.sym] = true;
        else
          std::cout << "Dropped unrecognized keycode " << event.key.keysym.sym << "." << std::endl;
        break;
      case SDL_KEYUP:
        if (event.key.keysym.sym < key_count)
          _keys[event.key.keysym.sym] = false;
        break;
    }
  }

  template<size_t key>
  bool is_key_down() const { return _keys[key]; }

  bool is_quit() const { return _quit; }

private:
  static constexpr int key_count = 322;
  bool _keys[key_count];
  bool _quit = false;
};
