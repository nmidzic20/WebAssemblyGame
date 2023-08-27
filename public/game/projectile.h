#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "collidable.h"

class Projectile : public Collidable {
    public:
        float speed;

        Projectile(float _x, float _y, float _width, float _height, float _collided, int _speed);
        
        virtual void draw(const Context *ctx) override;
        virtual void handle_collision(Context *ctx) override;
};

#endif

