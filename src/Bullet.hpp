#pragma once

#include "../libs/raylib/src/raylib.h"

class Bullet {
    public:
        bool isDead;
        Bullet(bool enemy, Color color, Vector3 position, Vector3 velocity);
        void drawBullet();
        void updateBullet(float deltaTime);
        float timeElapsed;
        bool enemy;

        Vector3 position;
    private:
        Vector3 _velocity;
        Color _color;
};
