#include "Bullet.hpp"
#include "../libs/raylib/src/raymath.h"
#include <iostream>

Bullet::Bullet(Color color, Vector3 position, Vector3 velocity) {
    this->_color = color;
    this->_position = position;
    this->_velocity = velocity;
    this->isDead = false;
    this->timeElapsed = 0;
}

void Bullet::drawBullet() {
    DrawCylinderEx(this->_position, Vector3Add(this->_position, this->_velocity), 0, 0.09, 1, this->_color);
}

void Bullet::updateBullet(float deltaTime) {
    this->_position = Vector3Add(this->_position, Vector3Scale(this->_velocity, deltaTime*100));
    this->timeElapsed += deltaTime;

    if (timeElapsed > 1) {
        this->isDead = true;
    }
}

