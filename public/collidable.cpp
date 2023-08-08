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
    SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 128, 255);
    SDL_RenderDrawLine(ctx->renderer, x, y, x + width / 2, y - height);
    SDL_RenderDrawLine(ctx->renderer, x + width / 2, y - height, x + width, y);
    SDL_RenderDrawLine(ctx->renderer, x, y, x + width, y);
}

void Collidable::handle_collision(context *ctx) {
    SDL_Rect cubeRect = {
        static_cast<int>(ctx->cube_position.x - ctx->cube_size / 2),
        static_cast<int>(ctx->cube_position.y - ctx->cube_size / 2),
        static_cast<int>(ctx->cube_size), static_cast<int>(ctx->cube_size)};

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
