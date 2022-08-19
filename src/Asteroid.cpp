#include "./Asteroid.hpp"
#include "../libs/raylib/src/raymath.h"

Asteroid::Asteroid(Vector3 position, Vector3 velocity) {
    this->position = position;
    this->velocity = velocity;
}

void Asteroid::draw() {
    DrawSphereEx(this->position, this->radius, this->rings, this->slices, BLACK);
    DrawSphereWires(this->position, this->radius, this->rings, this->slices, GRAY);
}

void Asteroid::update(float deltaTime) {
    this->position = Vector3Add(this->position, Vector3Scale(this->velocity, deltaTime));
}
