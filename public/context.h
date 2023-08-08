#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "collidable.h"

namespace Helper {
    bool check_collision(const SDL_Rect &rect1, const SDL_Rect &rect2);
}

struct context {
    const float WINDOW_WIDTH = 1000; 
    const float WINDOW_HEIGHT = 600; 
    const int NUMBER_COLLIDABLES = 5;
    const float PROJECTILE_COOLDOWN_TIME = 0.2f;

    struct Point {
        float x, y, z;
    };

    SDL_Window* window;
    SDL_Renderer* renderer;

    float cube_size = 50.0f;
    Point cube_position;
    
    std::vector<SDL_Texture*> images;
    int background_image, background_image_width, background_image_height;

    float background_offset = 0.0f;
    float scroll_speed = 1.0f;

    std::vector<Collidable *> collidables;

    float lastProjectileFiredTime = 0.0f;

    bool prevCollision = false;

    int score = 0;
    int lives = 10;
    int font_size = 32;
    TTF_Font *font;

    context() : cube_position{ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f } {}

    ~context() {
        for (Collidable* collidable : collidables) {
            delete collidable;
        }
    }
};

#endif