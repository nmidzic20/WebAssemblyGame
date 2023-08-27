#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "enemy.h"
#include "context.h"

Enemy::Enemy(float _x, float _y, float _width, float _height, float _collided, int _lives)
    : Collidable(_x, _y, _width, _height, _collided), lives(_lives) {}

void Enemy::draw(const Context *ctx) {
   
    SDL_Texture *enemyTexture = IMG_LoadTexture(ctx->renderer, "../assets/images/enemy.png");

    if (enemyTexture) {
        SDL_Rect destRect = {
            static_cast<int>(x), static_cast<int>(y - height),
            static_cast<int>(width), static_cast<int>(height)
        };

        SDL_RenderCopy(ctx->renderer, enemyTexture, nullptr, &destRect);
        SDL_DestroyTexture(enemyTexture);
    }
}

void Enemy::handle_collision(Context *ctx) {
    SDL_Rect cubeRect = {
        static_cast<int>(ctx->player_position.x - ctx->player_size / 2),
        static_cast<int>(ctx->player_position.y - ctx->player_size / 2),
        static_cast<int>(ctx->player_size), static_cast<int>(ctx->player_size)};

    if (!collided) {
        SDL_Rect collidableRect = {
            static_cast<int>(x), static_cast<int>(y - height),
            static_cast<int>(width), static_cast<int>(height)};

        if (Helper::check_collision(cubeRect, collidableRect)) {
            ctx->prevCollision = true;
            --ctx->lives;
            collided = true;
        } else {
            ctx->prevCollision = false;
        }
    }
}
