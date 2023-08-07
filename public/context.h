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
            bool collided = false;

            virtual void updatePosition(float scroll_speed) {
                x -= scroll_speed;
            }

            virtual void draw(const context *ctx) {
                SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 128, 255);
                SDL_RenderDrawLine(ctx->renderer, x, y, x + width/2, y - height);
                SDL_RenderDrawLine(ctx->renderer, x + width/2, y - height, x + width, y);
                SDL_RenderDrawLine(ctx->renderer, x, y, x + width, y);
            }

            virtual void handleCollision(context *ctx) {
                SDL_Rect cubeRect = {static_cast<int>(ctx->cube_position.x - ctx->cube_size / 2), static_cast<int>(ctx->cube_position.y - ctx->cube_size / 2), static_cast<int>(ctx->cube_size), static_cast<int>(ctx->cube_size)};

                if (!collided) {
                    SDL_Rect collidableRect = {static_cast<int>(x), static_cast<int>(y - height), static_cast<int>(width), static_cast<int>(height)};
                    
                    if (Helper::check_collision(cubeRect, collidableRect)) {
                        
                        ctx->score++;
                        x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
                        y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
                        
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

    class Enemy : public Collidable {
        public:
            int lives;

            virtual void draw(const context *ctx) override {
                SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255);
                SDL_RenderDrawLine(ctx->renderer, x, y, x + width/2, y - height);
                SDL_RenderDrawLine(ctx->renderer, x + width/2, y - height, x + width, y);
                SDL_RenderDrawLine(ctx->renderer, x, y, x + width, y);
            }

            virtual void handleCollision(context *ctx) override {
                SDL_Rect cubeRect = {static_cast<int>(ctx->cube_position.x - ctx->cube_size / 2), static_cast<int>(ctx->cube_position.y - ctx->cube_size / 2), static_cast<int>(ctx->cube_size), static_cast<int>(ctx->cube_size)};

                if (!collided) {
                    SDL_Rect collidableRect = {static_cast<int>(x), static_cast<int>(y - height), static_cast<int>(width), static_cast<int>(height)};
                    
                    if (Helper::check_collision(cubeRect, collidableRect)) {
                  
                        ctx->prevCollision = true;
                        if (--ctx->lives <= 0) {
                            ctx->scroll_speed = 0.0f;
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
            //bool active;

            virtual void draw(const context *ctx) override {
                //if (active) {
                    x += speed;

                    SDL_SetRenderDrawColor(ctx->renderer, 255, 165, 0, 255);
                    SDL_Rect projectileRect = {static_cast<int>(x), static_cast<int>(y), 20, 5};
                    SDL_RenderFillRect(ctx->renderer, &projectileRect);

                //}
            }

            virtual void handleCollision(context *ctx) override {
                //if (active) {
                    SDL_Rect projectileRect = {static_cast<int>(x), static_cast<int>(y), 20, 5};

                    for (Collidable *collidable : ctx->collidables) {
                        if (Enemy *enemy = dynamic_cast<Enemy *>(collidable)) {

                            //if (!collided) {

                                if (enemy->lives > 0) {
                                    SDL_Rect collidableRect = {static_cast<int>(collidable->x), static_cast<int>(collidable->y - collidable->height), static_cast<int>(collidable->width), static_cast<int>(collidable->height)};

                                    if (Helper::check_collision(projectileRect, collidableRect)) {
                                        enemy->lives--;
                                        //active = false;
                                        cout << "enemy collided, lives now " << enemy->lives << endl;

                                        // Remove projectile which hit enemy
                                        auto iterator = std::find(ctx->collidables.begin(), ctx->collidables.end(), this);
                                        if (iterator != ctx->collidables.end()) {
                                            cout << "TESR" << endl;
                                            ctx->collidables.erase(iterator);
                                        }

                                        if (enemy->lives <= 0) {
                                            cout << "Lives 0 or below" << endl;
                                            enemy->x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
                                            enemy->y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
                                            enemy->lives = 3;
                                        }
                                    }
                                }

                            /*    collided = true;
                            } else {
                                // Projectile is not colliding with this collidable
                                // This is to re-enable collision with the same collidable once the cube has already collided with it, and that collidable is still rendered on-screen
                                ctx->prevCollision = false;
                            }*/
                        }
                    }

                    // Remove projectiles which are out of window bounds
                    if (x > ctx->WINDOW_WIDTH) {
                        auto iterator = std::find(ctx->collidables.begin(), ctx->collidables.end(), this);
                        if (iterator != ctx->collidables.end()) {
                            ctx->collidables.erase(iterator);
                            delete this;
                        }
                    }
                    
                //}
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

    vector<Collidable *> collidables;
    //vector<Projectile> projectiles;
    float lastProjectileFiredTime = 0.0f;

    bool prevCollision = false;

    int score = 0;
    int lives = 10;
    int font_size = 32;
    TTF_Font *font;

    context() : cube_position{ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f }, is_yellow(false) {}

    ~context() {
        for (Collidable* collidable : collidables) {
            delete collidable;
        }
    }
};

#endif