#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "game/context.h"
#include "game/renderer.h"
#include "game/helper.h"
#include "game/collidable.h"
#include "game/enemy.h"
#include "game/projectile.h"

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
    bool gameStarted = false;

    EMSCRIPTEN_KEEPALIVE
    void start_game(context *ctx) {
        cout << "Called startgame" << endl;
        gameStarted = true;
    }
    /*Cannot this way due to asynchronous nature of doing main_loop not registering this change in ctx->game_state
    EMSCRIPTEN_KEEPALIVE
    void startGame(context *ctx) {
        cout << "Called startgame" << endl;
        if (ctx->game_state != context::GameState::GAMEPLAY)
            ctx->game_state = context::GameState::GAMEPLAY;
        EM_ASM(Module.startGame = false;);
        cout << "Game state is " << (ctx->game_state) << endl;
    }*/

    EMSCRIPTEN_KEEPALIVE
    void restart_game(context *ctx) {
        ctx->lives = 10;
        ctx->score = 0;
        ctx->scroll_speed = 1.0f;
        ctx->score_sent = false;

        EM_ASM(
            Module.gameDataStored = false;
            document.getElementById("username-input").style.display = "block";
            document.getElementById("submit-button").style.display = "block";
        );
    }

    EMSCRIPTEN_KEEPALIVE
    void set_avatar(context *ctx, int avatar_index) {
        ctx->avatar_selected = avatar_index;
        cout << "Avatar " << ctx->avatar_selected << endl;
    }

    EMSCRIPTEN_KEEPALIVE
    void handle_input(int key, int state, context *ctx) {
        
        switch (key) {
            case ' ':
                init_projectiles(ctx);
                break;
            case 38: // Up arrow code
            case 'W':
            case 'w':
                ctx->player_position.y = max(ctx->player_position.y - 10, ctx->player_size / 2);
                break;
            case 40: // Down arrow code
            case 'S':
            case 's':
                ctx->player_position.y = min(ctx->player_position.y + 10, ctx->WINDOW_HEIGHT - ctx->player_size / 2);
                break;
            case 37: // Left arrow code
            case 'A':
            case 'a':
                ctx->player_position.x = max(ctx->player_position.x - 10, ctx->player_size / 2);
                break;
            case 39: // Right arrow code
            case 'D':
            case 'd':
                ctx->player_position.x = min(ctx->player_position.x + 10, ctx->WINDOW_WIDTH - ctx->player_size / 2);
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
            ctx->player_position.x,
            ctx->player_position.y,
            20.0f,
            5.0f,
            false,
            5.0f
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
    Renderer::draw_player(ctx);
    Renderer::draw_collidables(ctx);

    if (Helper::is_game_over(ctx)) {
        Renderer::draw_game_over(ctx);
    } else {
        Renderer::draw_score(ctx);
    }
}

void main_loop(void *arg) {
    context *ctx = static_cast<context*>(arg);

    SDL_SetRenderDrawColor(ctx->renderer, 160, 0, 200, 255);
    SDL_RenderClear(ctx->renderer);

    if (gameStarted) {
        ctx->game_state = context::GameState::GAMEPLAY;
        gameStarted = false;
    }

    if (ctx->game_state == context::GameState::GAMEPLAY) {
        render_frame(ctx);
        handle_collisions(ctx);
    }

    SDL_RenderPresent(ctx->renderer);

    update_background_offset(ctx);
    update_collidables(ctx);

    if (Helper::is_game_over(ctx) && !ctx->score_sent) {
        ctx->score_sent = !ctx->score_sent;
        EM_ASM_({
            Module.playerScore = $0;
        }, ctx->score);
    }

    if (Helper::is_game_over(ctx))
        EM_ASM(
            var usernameContainer = document.getElementById("username-container");
            if (usernameContainer.style.display != "flex" && Module.gameDataStored === false) {
                usernameContainer.style.display = "flex";
            }
        );
    else
        EM_ASM(
            var usernameContainer = document.getElementById("username-container");
            if (usernameContainer.style.display != "none") {
                usernameContainer.style.display = "none";
            }
        );
}

int main(int argc, char *argv[]) {
    context ctx;

    SDL_Init(SDL_INIT_VIDEO);
    ctx.window = SDL_CreateWindow("Endless Runner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ctx.WINDOW_WIDTH, ctx.WINDOW_HEIGHT, 0);
    ctx.renderer = SDL_CreateRenderer(ctx.window, -1, SDL_RENDERER_ACCELERATED);

    EM_ASM({
        Module.onGameLoaded($0, $1);
    }, ctx.WINDOW_WIDTH, ctx.WINDOW_HEIGHT);

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
            Module.ccall('handle_input', 'void', ['int', 'int', 'number'], [event.keyCode, 1, $0]);
        };

        document.addEventListener('keydown', Module.handleEvent);

        Module.context = $0;
    , &ctx);

    emscripten_set_main_loop_arg(main_loop, &ctx, -1, 1);

    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}
