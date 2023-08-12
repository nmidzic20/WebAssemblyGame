#ifndef HELPER_H
#define HELPER_H

#include <SDL2/SDL.h>
#include "context.h"

class Collidable;

namespace Helper {
    bool check_collision(const SDL_Rect &rect1, const SDL_Rect &rect2);
    bool is_outside_window_bounds(Collidable *collidable);
    void reset_collided_flag(const context *ctx);
    bool is_game_over(const context *ctx);
}

#endif