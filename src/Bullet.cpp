#include "Bullet.hpp"
#include "../libs/raylib/src/raymath.h"
#include <iostream>

Bullet::Bullet(bool enemy, Color color, Vector3 position, Vector3 velocity) {
    this->color = color;
    this->position = position;
    this->velocity = velocity;
    this->isDead = false;
    this->timeElapsed = 0;
    this->isEnemy = enemy;
}

void Bullet::draw() {
    DrawCylinderEx(this->position,
                   Vector3Add(this->position, Vector3Scale(Vector3Normalize(this->velocity), 2)),
                   0, 0.09, 1, this->color);
}

void Bullet::update(float deltaTime) {
    this->position = Vector3Add(this->position, Vector3Scale(this->velocity, deltaTime));
    this->timeElapsed += deltaTime;

    if (timeElapsed > 1) {
        this->isDead = true;
    }
}

