#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <emscripten.h> // g++/gcc does not know where this header is, emcc does, so this line should be here only if compiling with emscripten

using namespace std;

struct context {
    const float WINDOW_WIDTH = 1000; 
    const float WINDOW_HEIGHT = 600; 

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
            bool collided = false;
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

    vector<Collidable> coins;
    vector<Collidable> enemies;

    bool prevCollision = false;

    int score = 0;
    int lives = 10;
    int font_size = 32;
    TTF_Font *font;

    context() : cube_position{ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f }, is_yellow(false) {}
};

int load_image(string image_path, context *ctx) {
    ctx->images.push_back(IMG_LoadTexture(ctx->renderer, image_path.c_str()));
    return ctx->images.size()-1;
}

void draw_image(int img, const context *ctx) {
    SDL_RenderCopy(ctx->renderer, ctx->images[img], nullptr, nullptr);
}

void draw_background(const context *ctx) {
    SDL_Rect dest_rect;
    dest_rect.x = -static_cast<int>(ctx->background_offset) % ctx->background_image_width;
    dest_rect.y = 0;
    dest_rect.w = ctx->background_image_width;
    dest_rect.h = ctx->background_image_height;

    SDL_RenderCopy(ctx->renderer, ctx->images[ctx->background_image], nullptr, &dest_rect);
}

void draw_cube(const context *ctx) {
    SDL_Rect rect;
    rect.w = ctx->cube_size;
    rect.h = ctx->cube_size;
    rect.x = ctx->cube_position.x - ctx->cube_size / 2;
    rect.y = ctx->cube_position.y - ctx->cube_size / 2;
 
    if (ctx->is_yellow) {
        SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255); 
    }
    
    SDL_RenderFillRect(ctx->renderer, &rect);
}

void draw_collidables(const context *ctx) {
    SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 128, 255);

    for (const auto &coin : ctx->coins) {
        SDL_RenderDrawLine(ctx->renderer, coin.x, coin.y, coin.x + coin.width/2, coin.y - coin.height);
        SDL_RenderDrawLine(ctx->renderer, coin.x + coin.width/2, coin.y - coin.height, coin.x + coin.width, coin.y);
        SDL_RenderDrawLine(ctx->renderer, coin.x, coin.y, coin.x + coin.width, coin.y);
    }

    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255);

    for (const auto &enemy : ctx->enemies) {
        SDL_RenderDrawLine(ctx->renderer, enemy.x, enemy.y, enemy.x + enemy.width/2, enemy.y - enemy.height);
        SDL_RenderDrawLine(ctx->renderer, enemy.x + enemy.width/2, enemy.y - enemy.height, enemy.x + enemy.width, enemy.y);
        SDL_RenderDrawLine(ctx->renderer, enemy.x, enemy.y, enemy.x + enemy.width, enemy.y);
    }
}

void draw_text(const context *ctx) {
    string score_text = "Score: " + to_string(ctx->score);
    string lives_text = "Lives: " + to_string(ctx->lives);

    const char *s_text = score_text.c_str();
    SDL_Surface *s_surface = TTF_RenderText_Solid(ctx->font, s_text, {255, 165, 0, 255});
    SDL_Texture *s_texture = SDL_CreateTextureFromSurface(ctx->renderer, s_surface);
    SDL_Rect s_destRect = {10, 10, s_surface->w, s_surface->h};
    SDL_FreeSurface(s_surface);
    SDL_RenderCopy(ctx->renderer, s_texture, NULL, &s_destRect);
    SDL_DestroyTexture(s_texture);

    const char *l_text = lives_text.c_str();
    SDL_Surface *l_surface = TTF_RenderText_Solid(ctx->font, l_text, {255, 165, 0, 255});
    SDL_Texture *l_texture = SDL_CreateTextureFromSurface(ctx->renderer, l_surface);
    SDL_Rect l_destRect = {10, 50, l_surface->w, l_surface->h};
    SDL_FreeSurface(l_surface);
    SDL_RenderCopy(ctx->renderer, l_texture, NULL, &l_destRect);
    SDL_DestroyTexture(l_texture);
}

void update_collidables(context *ctx) {

    for (auto &coin : ctx->coins) {
        coin.x -= ctx->scroll_speed;
        if (coin.x + coin.width < 0.0f) {
            // If the coin has scrolled completely out of view, reset its position on the right side with new random value
            coin.x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
            coin.y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
        }
    }
    for (auto &enemy : ctx->enemies) {
        enemy.x -= ctx->scroll_speed;
        if (enemy.x + enemy.width < 0.0f) {
            enemy.x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
            enemy.y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));

            enemy.collided = false;
        }
    }
}

void update_background_offset(context *ctx) {
    ctx->background_offset += ctx->scroll_speed;
    if (ctx->background_offset >= ctx->background_image_width - ctx->WINDOW_WIDTH) {
        ctx->background_offset = 0.0f;
    }
}

bool check_collision(const SDL_Rect& rect1, const SDL_Rect& rect2) {
    return (
        rect1.x < rect2.x + rect2.w &&
        rect1.x + rect1.w > rect2.x &&
        rect1.y < rect2.y + rect2.h &&
        rect1.y + rect1.h > rect2.y
    );
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void handleInput(int key, int state, context* ctx) {
        switch (key) {
            case 'M':
            case 'm':
                //if (state == 1)
                    ctx->is_yellow = !ctx->is_yellow;
                break;
            case 38:
            case 'W':
            case 'w':
                //if (state == 1)
                    ctx->cube_position.y = max(ctx->cube_position.y - 10, ctx->cube_size / 2);
                break;
            case 40:
            case 'S':
            case 's':
                //if (state == 1)
                    ctx->cube_position.y = min(ctx->cube_position.y + 10, ctx->WINDOW_HEIGHT - ctx->cube_size / 2);
                break;
            case 37:
            case 'A':
            case 'a':
                //if (state == 1)
                    ctx->cube_position.x = max(ctx->cube_position.x - 10, ctx->cube_size / 2);
                break;
            case 39:
            case 'D':
            case 'd':
                //if (state == 1)
                    ctx->cube_position.x = min(ctx->cube_position.x + 10, ctx->WINDOW_WIDTH - ctx->cube_size / 2);
                break;
        }
    }
    
}

void main_loop(void *arg) {
    context *ctx = static_cast<context*>(arg);

    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 255, 255);
    SDL_RenderClear(ctx->renderer);

    draw_background(ctx);

    if (ctx->lives > 0) {
        draw_cube(ctx);
    }

    draw_collidables(ctx);
    draw_text(ctx);

    // Check for collision between cube and each coin
    SDL_Rect cubeRect = {static_cast<int>(ctx->cube_position.x - ctx->cube_size / 2), static_cast<int>(ctx->cube_position.y - ctx->cube_size / 2), static_cast<int>(ctx->cube_size), static_cast<int>(ctx->cube_size)};
    for (auto &coin : ctx->coins) {
        SDL_Rect coinRect = {static_cast<int>(coin.x), static_cast<int>(coin.y - coin.height), static_cast<int>(coin.width), static_cast<int>(coin.height)};
        
        if (check_collision(cubeRect, coinRect)) {
            ctx->score++;
            coin.x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
            coin.y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
        }
    }
    for (auto &enemy : ctx->enemies) {

        if (!enemy.collided) { // Only check for collisions if the enemy hasn't collided before
            SDL_Rect enemyRect = {static_cast<int>(enemy.x), static_cast<int>(enemy.y - enemy.height), static_cast<int>(enemy.width), static_cast<int>(enemy.height)};
            
            if (check_collision(cubeRect, enemyRect)) {  

                ctx->prevCollision = true;      
                    
                // Remove a life only if not colliding with the same object    
                if (--ctx->lives <= 0) {
                    ctx->scroll_speed = 0.0f;
                }

                // Set the collided flag for this enemy to true to block further detections
                enemy.collided = true;

            }
            else {
                // Cube is not colliding with this enemy
                ctx->prevCollision = false;
            }

        }
    }

    // Reset the collided flag for the enemy if the cube is not colliding with it anymore
    if (!ctx->prevCollision) {
        for (auto &enemy : ctx->enemies) {
            SDL_Rect enemyRect = {static_cast<int>(enemy.x), static_cast<int>(enemy.y - enemy.height), static_cast<int>(enemy.width), static_cast<int>(enemy.height)};
            if (!check_collision(cubeRect, enemyRect)) {
                enemy.collided = false;
            }
        }
    }

    SDL_RenderPresent(ctx->renderer);

    update_background_offset(ctx);
    update_collidables(ctx);
}

int main(int argc, char *argv[]) {
    context ctx;

    SDL_Init(SDL_INIT_VIDEO);
    ctx.window = SDL_CreateWindow("Endless Runner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ctx.WINDOW_WIDTH, ctx.WINDOW_HEIGHT, 0);
    ctx.renderer = SDL_CreateRenderer(ctx.window, -1, SDL_RENDERER_ACCELERATED);

    ctx.background_image = load_image("./assets/image.png", &ctx);
    SDL_QueryTexture(ctx.images[ctx.background_image], nullptr, nullptr, &ctx.background_image_width, &ctx.background_image_height);

    TTF_Init();
    ctx.font = TTF_OpenFont("./assets/Ranchers-Regular.ttf", ctx.font_size);

    for (int i = 0; i < 5; i++) {
        context::Collidable coin;
        coin.x = static_cast<float>(rand() % static_cast<int>(ctx.WINDOW_WIDTH));
        coin.y = static_cast<float>(rand() % static_cast<int>(ctx.WINDOW_HEIGHT));
        coin.enemy = false;
        ctx.coins.push_back(coin);

        context::Collidable enemy;
        enemy.x = static_cast<float>(rand() % static_cast<int>(ctx.WINDOW_WIDTH));
        enemy.y = static_cast<float>(rand() % static_cast<int>(ctx.WINDOW_HEIGHT));
        enemy.enemy = true;
        ctx.enemies.push_back(enemy);
    }

    bool running = true;
    SDL_Event event;

    EM_ASM(
        Module.handleEvent = function(event) {
            if (event.type === "keydown") {
                Module.ccall('handleInput', 'void', ['int', 'int', 'number'], [event.keyCode, 1, $0]);
            }
        };

        document.addEventListener('keydown', Module.handleEvent);
    , &ctx);

    emscripten_set_main_loop_arg(main_loop, &ctx, -1, 1);

    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}
