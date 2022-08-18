#include "Bullet.hpp"
#include "../libs/raylib/src/raymath.h"
#include <iostream>

Bullet::Bullet(bool enemy, Color color, Vector3 position, Vector3 velocity) {
    this->_color = color;
    this->position = position;
    this->_velocity = velocity;
    this->isDead = false;
    this->timeElapsed = 0;
    this->enemy = enemy;
}

void Bullet::drawBullet() {
    DrawCylinderEx(this->position,
                   Vector3Add(this->position, Vector3Scale(Vector3Normalize(this->_velocity), 2)),
                   0, 0.09, 1, this->_color);
}

void Bullet::updateBullet(float deltaTime) {
    this->position = Vector3Add(this->position, Vector3Scale(this->_velocity, deltaTime));
    this->timeElapsed += deltaTime;

    if (timeElapsed > 1) {
        this->isDead = true;
    }
}

