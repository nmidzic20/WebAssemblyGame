#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "context.h"
#include "renderer.h"
#include "helper.h"
#include "collidable.h"
#include "enemy.h"
#include "projectile.h"

// g++/gcc does not know where this header is, emcc does, so this line should be here only if compiling with emscripten
#include <emscripten.h>

using namespace std;

int load_image(string image_path, context *ctx);
void init_collidables(context *ctx);
void init_projectiles(context *ctx);
void update_collidables(context *ctx);
void update_background_offset(context *ctx);
void handle_collisions(context *ctx);
void render_frame(context *ctx);

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void handleInput(int key, int state, context *ctx) {
        switch (key) {
            case ' ':
                init_projectiles(ctx);
                break;
            case 38: // Up arrow code
            case 'W':
            case 'w':
                ctx->cube_position.y = max(ctx->cube_position.y - 10, ctx->cube_size / 2);
                break;
            case 40: // Down arrow code
            case 'S':
            case 's':
                ctx->cube_position.y = min(ctx->cube_position.y + 10, ctx->WINDOW_HEIGHT - ctx->cube_size / 2);
                break;
            case 37: // Left arrow code
            case 'A':
            case 'a':
                ctx->cube_position.x = max(ctx->cube_position.x - 10, ctx->cube_size / 2);
                break;
            case 39: // Right arrow code
            case 'D':
            case 'd':
                ctx->cube_position.x = min(ctx->cube_position.x + 10, ctx->WINDOW_WIDTH - ctx->cube_size / 2);
                break;
        }
    }
}

int load_image(string image_path, context* ctx) {
    SDL_Texture* texture = IMG_LoadTexture(ctx->renderer, image_path.c_str());
    if (!texture) {
        throw runtime_error("Failed to load image: " + image_path + "\nError: " + SDL_GetError());
    }

    ctx->images.push_back(texture);
    return ctx->images.size() - 1;
}

void init_collidables(context *ctx) {
    for (int i = 0; i < ctx->NUMBER_COLLIDABLES; i++) {
        Collidable *coin = new Collidable(
            static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH)),
            static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT)),
            50.0f,
            50.0f,
            false
        );
        ctx->collidables.push_back(coin);

        Enemy *enemy = new Enemy(
            static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH)),
            static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT)),
            50.0f,
            50.0f,
            false,
            3
        );
        ctx->collidables.push_back(enemy);
    }
}

void init_projectiles(context *ctx) {
    float currentTime = SDL_GetTicks() / 1000.0f;
    if (currentTime - ctx->lastProjectileFiredTime >= ctx->PROJECTILE_COOLDOWN_TIME) {
        Projectile *projectile = new Projectile(
            ctx->cube_position.x,
            ctx->cube_position.y,
            20.0f,
            5.0f,
            false,
            5.0f
            //projectile->active = true;
        );
        ctx->collidables.push_back(projectile);

        ctx->lastProjectileFiredTime = currentTime;
    }
}

void update_collidables(context *ctx) {

    for (Collidable *collidable : ctx->collidables) {
        collidable->update_position(ctx->scroll_speed);
        if (Helper::is_outside_window_bounds(collidable)) {
            // If the collidable has scrolled completely out of view, reset its position on the right side with new random value
            collidable->reset_position(ctx);
        }
    }
}

void update_background_offset(context *ctx) {
    ctx->background_offset += ctx->scroll_speed;
    if (ctx->background_offset >= ctx->background_image_width - ctx->WINDOW_WIDTH) {
        ctx->background_offset = 0.0f;
    }
}

void handle_collisions(context *ctx) {
    
    for (Collidable *collidable : ctx->collidables) {
        collidable->update_position(ctx->scroll_speed);
        collidable->handle_collision(ctx);
    }
    Helper::reset_collided_flag(ctx);
}

void render_frame(context *ctx) {
    Renderer::draw_background(ctx);

    if (ctx->lives > 0) {
        Renderer::draw_cube(ctx);
    }

    Renderer::draw_collidables(ctx);
    Renderer::draw_text(ctx);
}

void main_loop(void *arg) {
    context *ctx = static_cast<context*>(arg);

    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 255, 255);
    SDL_RenderClear(ctx->renderer);

    render_frame(ctx);
    handle_collisions(ctx);

    SDL_RenderPresent(ctx->renderer);

    update_background_offset(ctx);
    update_collidables(ctx);
}

int main(int argc, char *argv[]) {
    context ctx;

    SDL_Init(SDL_INIT_VIDEO);
    ctx.window = SDL_CreateWindow("Endless Runner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ctx.WINDOW_WIDTH, ctx.WINDOW_HEIGHT, 0);
    ctx.renderer = SDL_CreateRenderer(ctx.window, -1, SDL_RENDERER_ACCELERATED);

    try {
        ctx.background_image = load_image("./assets/image.png", &ctx);
    }
    catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
    }
    SDL_QueryTexture(ctx.images[ctx.background_image], nullptr, nullptr, &ctx.background_image_width, &ctx.background_image_height);

    try {
        TTF_Init();
        ctx.font = TTF_OpenFont("./assets/Ranchers-Regular.ttf", ctx.font_size);
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
    }

    init_collidables(&ctx);

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
