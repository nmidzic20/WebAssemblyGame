#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

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
    };

    struct Projectile {
    float x;
    float y;
    float speed;
    bool active;
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
    vector<Projectile> projectiles;
    float lastProjectileFiredTime = 0.0f;

    bool prevCollision = false;

    int score = 0;
    int lives = 10;
    int font_size = 32;
    TTF_Font *font;

    context() : cube_position{ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f }, is_yellow(false) {}
};

#endif