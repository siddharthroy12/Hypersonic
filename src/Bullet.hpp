#pragma once

#include "../libs/raylib/src/raylib.h"
#include "./Entity.hpp"

class Bullet : public Entity {
    public:
        bool isDead;
        Bullet(bool enemy, Color color, Vector3 position, Vector3 velocity);
        void draw();
        void update(float deltaTime);
        float timeElapsed;
        bool isEnemy;
        Color color;
};
