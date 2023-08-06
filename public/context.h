#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "helper.h"

using namespace std;

struct context {
    const float WINDOW_WIDTH = 1000; 
    const float WINDOW_HEIGHT = 600; 
    const int NUMBER_COLLIDABLES = 5;
    const float PROJECTILE_COOLDOWN_TIME = 0.2f;

    struct Point {
        float x, y, z;
    };

    class Collidable {
        public:
            float x; // Horizontal coordinate of top left corner
            float y; // Vertical coordinate of top right corner
            float width = 50.0f;
            float height = 50.0f;

            bool enemy;
            int lives;
            bool collided = false;

            virtual void updatePosition(float scroll_speed) {
                x -= scroll_speed;
            }

            virtual void draw(const context *ctx) {
                if (enemy) {
                    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255);
                    SDL_RenderDrawLine(ctx->renderer, x, y, x + width/2, y - height);
                    SDL_RenderDrawLine(ctx->renderer, x + width/2, y - height, x + width, y);
                    SDL_RenderDrawLine(ctx->renderer, x, y, x + width, y);
                }
                else {
                    SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 128, 255);
                    SDL_RenderDrawLine(ctx->renderer, x, y, x + width/2, y - height);
                    SDL_RenderDrawLine(ctx->renderer, x + width/2, y - height, x + width, y);
                    SDL_RenderDrawLine(ctx->renderer, x, y, x + width, y);
                }
            }

            virtual void handleCollision(context *ctx) {
                SDL_Rect cubeRect = {static_cast<int>(ctx->cube_position.x - ctx->cube_size / 2), static_cast<int>(ctx->cube_position.y - ctx->cube_size / 2), static_cast<int>(ctx->cube_size), static_cast<int>(ctx->cube_size)};

                if (!collided) {
                    SDL_Rect collidableRect = {static_cast<int>(x), static_cast<int>(y - height), static_cast<int>(width), static_cast<int>(height)};
                    
                    if (Helper::check_collision(cubeRect, collidableRect)) {
                        // Logic for both coins and enemies when collision with cube happens
                        if (enemy) {
                            ctx->prevCollision = true;
                            if (--ctx->lives <= 0) {
                                ctx->scroll_speed = 0.0f;
                            }
                        } else {
                            ctx->score++;
                            x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
                            y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
                        }

                        // Reset the collided flag for this collidable to block further detections
                        collided = true;
                    } else {
                        // Cube is not colliding with this collidable
                        // This is to re-enable collision with the same collidable once the cube has already collided with it, and that collidable is still rendered on-screen
                        ctx->prevCollision = false;
                    }
                }
            }
    };

    class Projectile : public Collidable {
        public:
            float speed;
            bool active;

            virtual void draw(const context *ctx) override {
                if (active) {
                    x += speed;

                    SDL_SetRenderDrawColor(ctx->renderer, 255, 165, 0, 255);
                    SDL_Rect projectileRect = {static_cast<int>(x), static_cast<int>(y), static_cast<int>(x), static_cast<int>(y)};
                    SDL_RenderFillRect(ctx->renderer, &projectileRect);

                    if (x > ctx->WINDOW_WIDTH) {
                        //iterator = ctx->collidable.erase(iterator);
                        cout << "Projectile removed, out of bounds" << endl;
                        // Skip incrementing the iterator, since erase function automatically causes iterator to point to the next element
                        //continue; 
                    }
                }
            }

            virtual void handleCollision(context *ctx) override {
                if (active) {
                    SDL_Rect projectileRect = {static_cast<int>(x), static_cast<int>(y), 20, 5};

                    if (enemy && lives > 0) {
                        SDL_Rect collidableRect = {static_cast<int>(x), static_cast<int>(y - height), static_cast<int>(width), static_cast<int>(height)};

                        if (Helper::check_collision(projectileRect, collidableRect)) {
                            lives--;
                            active = false;

                            if (lives <= 0) {
                                x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
                                y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
                                lives = 3;
                            }
                        }
                    }
                    
                }
            }
        };

    SDL_Window* window;
    SDL_Renderer* renderer;

    float cube_size = 50.0f;
    Point cube_position;
    
    bool is_yellow;

    vector<SDL_Texture*> images;
    int background_image, background_image_width, background_image_height;

    float background_offset = 0.0f;
    float scroll_speed = 1.0f;

    vector<Collidable> collidables;
    //vector<Projectile> projectiles;
    float lastProjectileFiredTime = 0.0f;

    bool prevCollision = false;

    int score = 0;
    int lives = 10;
    int font_size = 32;
    TTF_Font *font;

    context() : cube_position{ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f }, is_yellow(false) {}
};

#endif