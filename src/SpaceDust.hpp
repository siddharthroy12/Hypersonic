#pragma once

#include "../libs/raylib/src/raylib.h"

#include <vector>

class SpaceDust {
    public:
        SpaceDust(float size, int count);

        void updateViewPosition(Vector3 viewPosition);
        void draw(Vector3 viewPosition, Vector3 velocity, bool drawDots) const;

    private:
        std::vector<Vector3> points;
        std::vector<Color> colors;
        float extent;
};
