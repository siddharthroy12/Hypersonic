#pragma once

#include "../libs/raylib/src/raylib.h"

class Bullet {
    public:
        bool isDead;
        Bullet(Color color, Vector3 position, Vector3 velocity);
        void drawBullet();
        void updateBullet(float deltaTime);
        float timeElapsed;
    private:
        Vector3 _position;
        Vector3 _velocity;
        Color _color;
};
