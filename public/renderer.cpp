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

void Renderer::draw_image(int img, const context* ctx) {
    SDL_RenderCopy(ctx->renderer, ctx->images[img], nullptr, nullptr);
}

void Renderer::draw_background(context *ctx) {

    if (Helper::is_game_over(ctx)) ctx->scroll_speed = 0.0f;

    SDL_Rect dest_rect;
    dest_rect.x = -static_cast<int>(ctx->background_offset) % ctx->background_image_width;
    dest_rect.y = 0;
    dest_rect.w = ctx->background_image_width;
    dest_rect.h = ctx->background_image_height;

    SDL_RenderCopy(ctx->renderer, ctx->images[ctx->background_image], nullptr, &dest_rect);
}

void Renderer::draw_cube(const context *ctx) {

    if (Helper::is_game_over(ctx)) return;

    SDL_Rect rect;
    rect.w = ctx->cube_size;
    rect.h = ctx->cube_size;
    rect.x = ctx->cube_position.x - ctx->cube_size / 2;
    rect.y = ctx->cube_position.y - ctx->cube_size / 2;

    SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255); 
    SDL_RenderFillRect(ctx->renderer, &rect);
}

void Renderer::draw_collidables(context *ctx) {

    for (Collidable *collidable : ctx->collidables) {
        collidable->draw(ctx);
    }
}

void Renderer::draw_score(const context *ctx) {
    /*string score_text = "Score: " + to_string(ctx->score);
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
    */
    context::Text score_text = { "Score: " + to_string(ctx->score), {255, 165, 0, 255}, { 10, 10 } };
    context::Text lives_text = { "Lives: " + to_string(ctx->lives), {255, 165, 0, 255}, { 10, 50 } };

    draw_text(ctx, score_text);
    draw_text(ctx, lives_text);
}

void Renderer::draw_game_over(context *ctx) {
    string game_over = "Game over!";
    string score_achieved = "Score achieved: " + to_string(ctx->score);
    string restart = "Restart";

    SDL_Color text_color = {165, 0, 255, 255};
    int textWidth, textHeight;

    TTF_SizeText(ctx->font, game_over.c_str(), &textWidth, &textHeight);
    context::Text text = { game_over, text_color, { (ctx->WINDOW_WIDTH - textWidth) / 2, (ctx->WINDOW_HEIGHT - 2.5f * textHeight) / 2 } };
    Renderer::draw_text(ctx, text);

    TTF_SizeText(ctx->font, score_achieved.c_str(), &textWidth, &textHeight);
    text = { score_achieved, text_color, { (ctx->WINDOW_WIDTH - textWidth) / 2, (ctx->WINDOW_HEIGHT - textHeight) / 2 } };
    Renderer::draw_text(ctx, text);

    TTF_SizeText(ctx->font, restart.c_str(), &textWidth, &textHeight);
    SDL_Rect restartRect = { static_cast<int>((ctx->WINDOW_WIDTH - textWidth) / 2), static_cast<int>((ctx->WINDOW_HEIGHT + 1.5f * textHeight) / 2), textWidth, textHeight };
    text = { restart, text_color, { (ctx->WINDOW_WIDTH - textWidth) / 2, (ctx->WINDOW_HEIGHT + 1.5f * textHeight) / 2 } };
    Renderer::draw_text(ctx, text);

    // Restart game when hovering over Restart text
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (mouseX >= restartRect.x && mouseX <= restartRect.x + textWidth &&
        mouseY >= restartRect.y && mouseY <= restartRect.y + textHeight) {
            ctx->lives = 10;
            ctx->score = 0;
            ctx->scroll_speed = 1.0f;
    }
}

void Renderer::draw_text(const context *ctx, context::Text text) {
    const char *p_text = text.text.c_str();
    SDL_Surface *surface = TTF_RenderText_Solid(ctx->font, p_text, text.color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);
    SDL_Rect destRect = {static_cast<int>(text.position.x), static_cast<int>(text.position.y), surface->w, surface->h};
    SDL_FreeSurface(surface);
    SDL_RenderCopy(ctx->renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}
