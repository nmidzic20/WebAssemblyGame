#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <emscripten.h> // g++/gcc does not know where this header is, emcc does, so this line should be here only if compiling with emscripten

using namespace std;

struct context {
    const float WINDOW_WIDTH = 1000; 
    const float WINDOW_HEIGHT = 600; 

    struct Point {
        float x, y, z;
    };

    SDL_Window* window;
    SDL_Renderer* renderer;

    float cube_size = 50.0f;
    Point cube_position;
    
    bool is_yellow;

    vector<SDL_Texture*> images;
    int background_image;
    int background_image_width, background_image_height;

    float background_offset = 0.0f;
    float scroll_speed = 1.0f;

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

void update_background_offset(context *ctx) {
    ctx->background_offset += ctx->scroll_speed;
    if (ctx->background_offset >= ctx->background_image_width - ctx->WINDOW_WIDTH) {
        ctx->background_offset = 0.0f;
    }
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
                //if (state == 1)
                    ctx->cube_position.y = max(ctx->cube_position.y - 10, ctx->cube_size / 2);
                break;
            case 40:
                //if (state == 1)
                    ctx->cube_position.y = min(ctx->cube_position.y + 10, ctx->WINDOW_HEIGHT - ctx->cube_size / 2);
                break;
            case 37:
                //if (state == 1)
                    ctx->cube_position.x = max(ctx->cube_position.x - 10, ctx->cube_size / 2);
                break;
            case 39:
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
    draw_cube(ctx);

    SDL_RenderPresent(ctx->renderer);

    update_background_offset(ctx);
}

int main(int argc, char *argv[]) {
    context ctx;

    SDL_Init(SDL_INIT_VIDEO);
    ctx.window = SDL_CreateWindow("Endless Runner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ctx.WINDOW_WIDTH, ctx.WINDOW_HEIGHT, 0);
    ctx.renderer = SDL_CreateRenderer(ctx.window, -1, SDL_RENDERER_ACCELERATED);

    ctx.background_image = load_image("./assets/image.png", &ctx);
    SDL_QueryTexture(ctx.images[ctx.background_image], nullptr, nullptr, &ctx.background_image_width, &ctx.background_image_height);

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
