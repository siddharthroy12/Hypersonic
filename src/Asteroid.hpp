#pragma once

#include "./Entity.hpp"
#include "../libs/raylib/src/raylib.h"


class Asteroid : public Entity {
    public:
        float radius = 1;
        int rings = 5;
        int slices = 4;
        Asteroid(Vector3 position, Vector3 velocity);
        void draw();
        void update(float deltaTime);
};
