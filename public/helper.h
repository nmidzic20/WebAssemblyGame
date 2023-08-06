#ifndef HELPER_H
#define HELPER_H

#include <SDL2/SDL.h>

namespace Helper {
    bool check_collision(const SDL_Rect &rect1, const SDL_Rect &rect2);
}

#endif