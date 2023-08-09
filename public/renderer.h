#ifndef RENDERER_H
#define RENDERER_H

#include "context.h"

namespace Renderer {
    void draw_image(int img, const context *ctx);
    void draw_background(context *ctx);
    void draw_cube(const context *ctx);
    void draw_collidables(context *ctx);
    void draw_score(const context *ctx);
    void draw_game_over(context *ctx);
    void draw_text(const context* ctx, context::Text text);
}

#endif
