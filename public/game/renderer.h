#ifndef RENDERER_H
#define RENDERER_H

#include "context.h"

namespace Renderer {
    void draw_image(int img, const Context *ctx);
    void draw_background(Context *ctx);
    void draw_player(const Context *ctx);
    void draw_collidables(Context *ctx);
    void draw_score(const Context *ctx);
    void draw_game_over(Context *ctx);
    void draw_text(const Context* ctx, Context::Text text);
}

#endif
