#include "collidable.h"
#include "helper.h"
#include <SDL2/SDL.h>

#include "context.h"
#include "enemy.h"

Collidable::Collidable(float _x, float _y, float _width, float _height, bool _collided)
    : x(_x), y(_y), width(_width), height(_height), collided(_collided) {}

void Collidable::reset_position(const context *ctx) {
    x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
    y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));

    collided = false;

    // Reset enemy lives (enemy might have had lives reduced by projectiles in previous image scroll)
    if (Enemy *enemy = dynamic_cast<Enemy *>(this)) {
        enemy->lives = 3;
    }
}

void Collidable::update_position(float scroll_speed) {
    x -= scroll_speed;
}

void Collidable::draw(const context *ctx) {
    SDL_Texture *pointTexture = IMG_LoadTexture(ctx->renderer, "../assets/images/point.png");

    if (pointTexture) {
        SDL_Rect destRect = {
            static_cast<int>(x), static_cast<int>(y - height),
            static_cast<int>(width), static_cast<int>(height)
        };

        SDL_RenderCopy(ctx->renderer, pointTexture, nullptr, &destRect);
        SDL_DestroyTexture(pointTexture);
    }
}

void Collidable::handle_collision(context *ctx) {
    SDL_Rect cubeRect = {
        static_cast<int>(ctx->player_position.x - ctx->player_size / 2),
        static_cast<int>(ctx->player_position.y - ctx->player_size / 2),
        static_cast<int>(ctx->player_size), static_cast<int>(ctx->player_size)};

    if (!collided) {
        SDL_Rect collidableRect = {
            static_cast<int>(x), static_cast<int>(y - height),
            static_cast<int>(width), static_cast<int>(height)};

        if (Helper::check_collision(cubeRect, collidableRect)) {
            ctx->score++;
            x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
            y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
            collided = true;
        } else {
            ctx->prevCollision = false;
        }
    }
}
