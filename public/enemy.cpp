#include "enemy.h"
#include <SDL2/SDL.h>

#include "context.h"

Enemy::Enemy(float _x, float _y, float _width, float _height, float _collided, int _lives)
    : Collidable(_x, _y, _width, _height, _collided), lives(_lives) {}

void Enemy::draw(const context *ctx) {
    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(ctx->renderer, x, y, x + width / 2, y - height);
    SDL_RenderDrawLine(ctx->renderer, x + width / 2, y - height, x + width, y);
    SDL_RenderDrawLine(ctx->renderer, x, y, x + width, y);
}

void Enemy::handle_collision(context *ctx) {
    SDL_Rect cubeRect = {
        static_cast<int>(ctx->cube_position.x - ctx->cube_size / 2),
        static_cast<int>(ctx->cube_position.y - ctx->cube_size / 2),
        static_cast<int>(ctx->cube_size), static_cast<int>(ctx->cube_size)};

    if (!collided) {
        SDL_Rect collidableRect = {
            static_cast<int>(x), static_cast<int>(y - height),
            static_cast<int>(width), static_cast<int>(height)};

        if (Helper::check_collision(cubeRect, collidableRect)) {
            ctx->prevCollision = true;
            if (--ctx->lives <= 0) {
                ctx->scroll_speed = 0.0f;
            }

            collided = true;
        } else {
            ctx->prevCollision = false;
        }
    }
}
