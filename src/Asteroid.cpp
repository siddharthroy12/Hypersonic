#include "./Asteroid.hpp"
#include "../libs/raylib/src/raymath.h"

Asteroid::Asteroid(Model model, Vector3 position, Vector3 velocity) {
    this->position = position;
    this->velocity = velocity;
    this->model = model;
    this->scale = 0;
}

void Asteroid::draw() {
    DrawModel(this->model, this->position, this->scale, {68, 68, 68, 225});
    DrawModelWires(this->model, this->position, this->scale, GRAY);
}

void Asteroid::update(float deltaTime) {
    this->position = Vector3Add(this->position, Vector3Scale(this->velocity, deltaTime));

    if (this->scale >= 1) {
        this->scale = 1;
    } else {
        this->scale += deltaTime * 0.7;
    }
}
