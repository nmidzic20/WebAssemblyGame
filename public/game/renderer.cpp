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

using namespace std;

void Renderer::draw_image(int img, const Context* ctx) {
    SDL_RenderCopy(ctx->renderer, ctx->images[img], nullptr, nullptr);
}

void Renderer::draw_background(Context *ctx) {

    if (Helper::is_game_over(ctx)) ctx->scroll_speed = 0.0f;

    SDL_Rect dest_rect;
    dest_rect.x = -static_cast<int>(ctx->background_offset) % ctx->background_image_width;
    dest_rect.y = 0;
    dest_rect.w = ctx->background_image_width;
    dest_rect.h = ctx->background_image_height;

    SDL_RenderCopy(ctx->renderer, ctx->images[ctx->background_image], nullptr, &dest_rect);
}

void Renderer::draw_player(const Context *ctx) {

    if (Helper::is_game_over(ctx)) return;

    SDL_Color color = { 255, 255, 255, 255 };

    if (ctx->avatar_selected == Context::Avatar::MODEL_1) {
        color = { 255, 255, 0, 255 };
    }
    else if (ctx->avatar_selected == Context::Avatar::MODEL_2) {
        color = { 90, 90, 90, 255 };
    }
    else if (ctx->avatar_selected == Context::Avatar::MODEL_3) {
        color = { 254, 44, 84, 255 };
    }
    
    SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a); 
    
    SDL_Texture *rocketTexture = IMG_LoadTexture(ctx->renderer, "../assets/images/player.png");
    if (rocketTexture) {
        SDL_SetTextureColorMod(rocketTexture, color.r, color.g, color.b);

        SDL_Rect destRect;
        destRect.w = ctx->player_size;
        destRect.h = ctx->player_size;
        destRect.x = ctx->player_position.x - ctx->player_size / 2;
        destRect.y = ctx->player_position.y - ctx->player_size / 2;

        SDL_RenderCopy(ctx->renderer, rocketTexture, nullptr, &destRect);
        SDL_SetTextureColorMod(rocketTexture, 255, 255, 255);
        SDL_DestroyTexture(rocketTexture);
    }
}

void Renderer::draw_collidables(Context *ctx) {

    for (Collidable *collidable : ctx->collidables) {
        collidable->draw(ctx);
    }
}

void Renderer::draw_score(const Context *ctx) {
    Context::Text score_text = { "Score: " + to_string(ctx->score), {255, 165, 0, 255}, { 10, 10 } };
    Context::Text lives_text = { "Lives: " + to_string(ctx->lives), {255, 165, 0, 255}, { 10, 50 } };

    draw_text(ctx, score_text);
    draw_text(ctx, lives_text);
}

void Renderer::draw_game_over(Context *ctx) {
    string game_over = "Game over!";
    string score_achieved = "Score achieved: " + to_string(ctx->score);

    SDL_Color text_color = {165, 0, 255, 255};
    int textWidth, textHeight;

    TTF_SizeText(ctx->font, game_over.c_str(), &textWidth, &textHeight);
    Context::Text text = { game_over, text_color, { (ctx->WINDOW_WIDTH - textWidth) / 2, 40.0f}};
    Renderer::draw_text(ctx, text);

    TTF_SizeText(ctx->font, score_achieved.c_str(), &textWidth, &textHeight);
    text = { score_achieved, text_color, { (ctx->WINDOW_WIDTH - textWidth) / 2, 50.0f + textHeight}};
    Renderer::draw_text(ctx, text);
}

void Renderer::draw_text(const Context *ctx, Context::Text text) {
    const char *p_text = text.text.c_str();
    SDL_Surface *surface = TTF_RenderText_Solid(ctx->font, p_text, text.color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);
    SDL_Rect destRect = {static_cast<int>(text.position.x), static_cast<int>(text.position.y), surface->w, surface->h};
    SDL_FreeSurface(surface);
    SDL_RenderCopy(ctx->renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}
