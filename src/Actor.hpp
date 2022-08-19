#pragma once

#include "../libs/raylib/src/raylib.h"
#include "./Entity.hpp"

class Actor : public Entity {
    public:
        Actor();
        Quaternion rotation;

        Vector3 getForward() const;
        Vector3 getBack() const;
        Vector3 getRight() const;
        Vector3 getLeft() const;
        Vector3 getUp() const;
        Vector3 getDown() const;

        Vector3 transformPoint(Vector3 point) const;
        void rotateLocalEuler(Vector3 axis, float degrees);
};
