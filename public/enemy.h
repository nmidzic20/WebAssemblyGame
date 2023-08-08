#ifndef ENEMY_H
#define ENEMY_H

#include "collidable.h"

class Enemy : public Collidable {
    public:
        int lives;

        Enemy(float _x, float _y, float _width, float _height, float _collided, int _lives);
        
        virtual void draw(const context *ctx) override;
        virtual void handle_collision(context *ctx) override;
};

#endif // ENEMY_H
