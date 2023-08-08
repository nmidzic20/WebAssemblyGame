#include "projectile.h"

#include "context.h"
#include "enemy.h"


Projectile::Projectile(float _x, float _y, float _width, float _height, float _collided, int _speed)
    : Collidable(_x, _y, _width, _height, _collided), speed(_speed) {}

void Projectile::draw(const context *ctx) {
    x += speed;

    SDL_SetRenderDrawColor(ctx->renderer, 255, 165, 0, 255);
    SDL_Rect projectileRect = {static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height)};
    SDL_RenderFillRect(ctx->renderer, &projectileRect);
}

void Projectile::handle_collision(context *ctx) {
    SDL_Rect projectileRect = {static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height)};

    for (Collidable *collidable : ctx->collidables) {
        if (Enemy *enemy = dynamic_cast<Enemy *>(collidable)) {

            if (enemy->lives > 0) {
                SDL_Rect collidableRect = {static_cast<int>(collidable->x), static_cast<int>(collidable->y - collidable->height), static_cast<int>(collidable->width), static_cast<int>(collidable->height)};

                if (Helper::check_collision(projectileRect, collidableRect)) {
                    enemy->lives--;

                    auto iterator = std::find(ctx->collidables.begin(), ctx->collidables.end(), this);
                    if (iterator != ctx->collidables.end()) {
                        ctx->collidables.erase(iterator);
                    }

                    if (enemy->lives <= 0) {
                        enemy->x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
                        enemy->y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
                        enemy->lives = 3;
                    }
                }
            }
        }
    }

    if (x > ctx->WINDOW_WIDTH) {
        auto iterator = std::find(ctx->collidables.begin(), ctx->collidables.end(), this);
        if (iterator != ctx->collidables.end()) {
            ctx->collidables.erase(iterator);
            delete this;
        }
    }
}
