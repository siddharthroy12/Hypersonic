#include "Actor.hpp"

#include "../libs/raylib/src/raymath.h"

Actor::Actor() {
    position = Vector3Zero();
    velocity = Vector3Zero();
    rotation = QuaternionIdentity();
}

Vector3 Actor::getForward() const {
    return Vector3RotateByQuaternion(
            Vector3{ 0, 0, 1 },
            rotation);
}

Vector3 Actor::getBack() const {
    return Vector3RotateByQuaternion(
            Vector3{ 0, 0, -1 },
            rotation);
}

Vector3 Actor::getRight() const {
    return Vector3RotateByQuaternion(
            Vector3{ -1, 0, 0 },
            rotation);
}

Vector3 Actor::getLeft() const {
    return Vector3RotateByQuaternion(
            Vector3{ 1, 0, 0 },
            rotation);
}

Vector3 Actor::getUp() const {
    return Vector3RotateByQuaternion(
            Vector3{ 0, 1, 0 },
            rotation);
}

Vector3 Actor::getDown() const
{
    return Vector3RotateByQuaternion(
            Vector3{ 0, -1, 0 },
            rotation);
}

Vector3 Actor::transformPoint(Vector3 point) const {
    auto mPos = MatrixTranslate(position.x, position.y, position.z);
    auto mRot = QuaternionToMatrix(rotation);
    auto matrix = MatrixMultiply(mRot, mPos);
    return Vector3Transform(point, matrix);
}

void Actor::rotateLocalEuler(Vector3 axis, float degrees) {
    auto radians = degrees * DEG2RAD;
    rotation = QuaternionMultiply(
            rotation,
            QuaternionFromAxisAngle(axis, radians));
}
