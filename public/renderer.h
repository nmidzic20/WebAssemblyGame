#ifndef RENDERER_H
#define RENDERER_H

#include "context.h"

namespace Renderer {
    void draw_image(int img, const context* ctx);
    void draw_background(const context* ctx);
    void draw_cube(const context* ctx);
    void draw_collidables(context* ctx);
    void draw_text(const context* ctx);
}

#endif
