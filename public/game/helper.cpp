#include <SDL2/SDL.h>
#include "context.h"
#include "helper.h"

bool Helper::check_collision(const SDL_Rect &rect1, const SDL_Rect &rect2) {
    return (
        rect1.x < rect2.x + rect2.w &&
        rect1.x + rect1.w > rect2.x &&
        rect1.y < rect2.y + rect2.h &&
        rect1.y + rect1.h > rect2.y
    );
}

bool Helper::is_outside_window_bounds(Collidable *collidable) {
    return collidable->x + collidable->width < 0.0f;
}

bool Helper::is_game_over(const Context *ctx) {
    return ctx->lives <= 0;
}

void Helper::reset_collided_flag(const Context *ctx) {
    SDL_Rect cubeRect = {static_cast<int>(ctx->player_position.x - ctx->player_size / 2), static_cast<int>(ctx->player_position.y - ctx->player_size / 2), static_cast<int>(ctx->player_size), static_cast<int>(ctx->player_size)};

    // Reset the collided flag for the collidable if the cube is not colliding with it anymore
    if (!ctx->prevCollision) {
        for (Collidable *collidable : ctx->collidables) {
            SDL_Rect collidableRect = {static_cast<int>(collidable->x), static_cast<int>(collidable->y - collidable->height), static_cast<int>(collidable->width), static_cast<int>(collidable->height)};
            if (!Helper::check_collision(cubeRect, collidableRect)) {
                collidable->collided = false;
            }
        }
    }
}

int Helper::load_image(std::string image_path, Context* ctx) {
    SDL_Texture* texture = IMG_LoadTexture(ctx->renderer, image_path.c_str());
    if (!texture) {
        throw std::runtime_error("Failed to load image: " + image_path + "\nError: " + SDL_GetError());
    }

    ctx->images.push_back(texture);
    return ctx->images.size() - 1;
}