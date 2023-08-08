#ifndef HELPER_H
#define HELPER_H

#include <SDL2/SDL.h>
#include "context.h"

namespace Helper {
    bool check_collision(const SDL_Rect &rect1, const SDL_Rect &rect2);
    bool is_outside_window_bounds(context::Collidable *collidable);
    void reset_collided_flag(const context *ctx);
}

#endif