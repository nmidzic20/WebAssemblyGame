#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "context.h"
#include "renderer.h"

using namespace std;

void Renderer::draw_image(int img, const context* ctx) {
    SDL_RenderCopy(ctx->renderer, ctx->images[img], nullptr, nullptr);
}

void Renderer::draw_background(const context* ctx) {
    SDL_Rect dest_rect;
    dest_rect.x = -static_cast<int>(ctx->background_offset) % ctx->background_image_width;
    dest_rect.y = 0;
    dest_rect.w = ctx->background_image_width;
    dest_rect.h = ctx->background_image_height;

    SDL_RenderCopy(ctx->renderer, ctx->images[ctx->background_image], nullptr, &dest_rect);
}

void Renderer::draw_cube(const context* ctx) {
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

void Renderer::draw_collidables(context* ctx) {

    for (context::Collidable *collidable : ctx->collidables) {
        collidable->draw(ctx);
        /*if (collidable.enemy) {
            SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255);
            SDL_RenderDrawLine(ctx->renderer, collidable.x, collidable.y, collidable.x + collidable.width/2, collidable.y - collidable.height);
            SDL_RenderDrawLine(ctx->renderer, collidable.x + collidable.width/2, collidable.y - collidable.height, collidable.x + collidable.width, collidable.y);
            SDL_RenderDrawLine(ctx->renderer, collidable.x, collidable.y, collidable.x + collidable.width, collidable.y);
        }
        else {
            SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 128, 255);
            SDL_RenderDrawLine(ctx->renderer, collidable.x, collidable.y, collidable.x + collidable.width/2, collidable.y - collidable.height);
            SDL_RenderDrawLine(ctx->renderer, collidable.x + collidable.width/2, collidable.y - collidable.height, collidable.x + collidable.width, collidable.y);
            SDL_RenderDrawLine(ctx->renderer, collidable.x, collidable.y, collidable.x + collidable.width, collidable.y);
        }*/
    }
}

void Renderer::draw_text(const context* ctx) {
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
