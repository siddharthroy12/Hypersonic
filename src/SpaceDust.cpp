#include "SpaceDust.hpp"

#include "../libs/raylib/src/raymath.h"
#include "../libs/raylib/src/rlgl.h"
#include <cmath>
#include <array>

inline float getPrettyBadRandomFloat(float min, float max) {
    auto value = static_cast<float>(GetRandomValue((int)min * 1000, (int)max * 1000));
    value /= 1000.f;
    return value;
}

SpaceDust::SpaceDust(float size, int count) {
    points = std::vector<Vector3>();
    points.reserve(count);
    extent = size * .5f;

    for (int i = 0; i < count; ++i) {
        auto point = Vector3{
            getPrettyBadRandomFloat(-extent, extent),
            getPrettyBadRandomFloat(-extent, extent),
            getPrettyBadRandomFloat(-extent, extent)
        };

        points.push_back(point);

        auto color = Color{
            (unsigned char)GetRandomValue(192, 255),
                (unsigned char)GetRandomValue(192, 255),
                (unsigned char)GetRandomValue(192, 255),
                255
        };
        colors.push_back(color);
    }
}

void SpaceDust::updateViewPosition(Vector3 viewPosition) {
    float size = extent * 2;
    for (auto& p : points) {
        while (p.x > viewPosition.x + extent)
            p.x -= size;
        while (p.x < viewPosition.x - extent)
            p.x += size;

        while (p.y > viewPosition.y + extent)
            p.y -= size;
        while (p.y < viewPosition.y - extent)
            p.y += size;

        while (p.z > viewPosition.z + extent)
            p.z -= size;
        while (p.z < viewPosition.z - extent)
            p.z += size;
    }
}

void SpaceDust::draw(Vector3 viewPosition, Vector3 velocity, bool drawDots) const {
    BeginBlendMode(BlendMode::BLEND_ADDITIVE);

    for (int i = 0; i < points.size(); ++i) {
        float distance = Vector3Distance(viewPosition, points[i]);

        float farLerp = Clamp(Normalize(distance, extent * .9f, extent), 0, 1);
        unsigned char farAlpha = (unsigned char)Lerp(255, 0, farLerp);

        const float cubeSize = 0.01f;

        if (drawDots) {
            DrawSphereWires(points[i],
                            cubeSize,
                            2, 4,
                            { colors[i].r, colors[i].g, colors[i].b, farAlpha });
        }

        DrawLine3D(Vector3Add(points[i], Vector3Scale(velocity, 0.02f)),
                   points[i],
                  { colors[i].r, colors[i].g, colors[i].b, farAlpha });
    }

    rlDrawRenderBatchActive();
    EndBlendMode();
}
