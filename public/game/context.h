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
        float x, y;
    };

    struct Text {
        std::string text;
        SDL_Color color;
        Point position;
    };

    enum GameState {
        START,
        GAMEPLAY
    };

    enum Avatar {
        CUBE,
        TRIANGLE
    };

    GameState game_state;

    SDL_Window *window;
    SDL_Renderer *renderer;

    float player_size = 50.0f;
    Point player_position;
    bool prevCollision = false;

    float lastProjectileFiredTime = 0.0f;
    
    std::vector<SDL_Texture*> images;
    int background_image, background_image_width, background_image_height;

    float background_offset = 0.0f;
    float scroll_speed = 1.0f;

    std::vector<Collidable *> collidables;

    int score = 0;
    int lives = 10;
    int font_size = 32;
    TTF_Font *font;

    bool score_sent;

    int avatar_selected;

    context() : player_position{ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 }, score_sent(false), game_state(GameState::START) {}

    ~context() {
        for (Collidable* collidable : collidables) {
            delete collidable;
        }
    }
};

#endif