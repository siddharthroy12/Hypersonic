#pragma once

#include "./Entity.hpp"
#include "../libs/raylib/src/raylib.h"


class Asteroid : public Entity {
    public:
        float radius = 1;
        int rings = 5;
        int slices = 4;
        float scale = 0;
        bool isDead = false;
        Model model;
        Asteroid(Model model, Vector3 position, Vector3 velocity);
        void draw();
        void update(float deltaTime);
};
