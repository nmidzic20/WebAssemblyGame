#ifndef COLLIDABLE_H
#define COLLIDABLE_H

class Context;

class Collidable {
    public:
        float x; // Horizontal coordinate of top left corner
        float y; // Vertical coordinate of top left corner
        float width, height;
        bool collided;

        Collidable(float _x, float _y, float _width, float _height, bool _collided);

        void reset_position(const Context *ctx);
        void update_position(float scroll_speed);
        virtual void draw(const Context *ctx);
        virtual void handle_collision(Context *ctx);
};

#endif

