#ifndef COLLIDABLE_H
#define COLLIDABLE_H

class context;

class Collidable {
    public:
        float x; // Horizontal coordinate of top left corner
        float y; // Vertical coordinate of top left corner
        float width;
        float height;
        bool collided;

        Collidable(float _x, float _y, float _width, float _height, bool _collided);

        void reset_position(const context *ctx);
        virtual void update_position(float scroll_speed);
        virtual void draw(const context *ctx);
        virtual void handle_collision(context *ctx);
};

#endif // COLLIDABLE_H

