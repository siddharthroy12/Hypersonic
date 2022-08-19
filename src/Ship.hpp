#pragma once

#include "Actor.hpp"

#include "../libs/raylib/src/raylib.h"

struct TrailRung {
    Vector3 leftPoint;
    Vector3 rightPoint;
    float timeToLive;
};

class Ship : public Actor {
    public:
        float inputForward = 0;
        float inputLeft = 0;
        float inputUp = 0;

        float inputPitchDown = 0;
        float inputRollRight = 0;
        float inputYawLeft = 0;

        float maxSpeed = 50;
        float throttleResponse = 10;
        float turnRate = 50;
        float turnResponse = 10;

        float length = 1.0f;
        float width = 1.0f;

        Color trailColor = DARKGREEN;
        bool isDead = false;
        bool isEnemy = false;

        Ship(Model model, bool isEnemy);

        void update(float deltaTime);
        void draw(bool showDebugAxes) const;
        void drawTrail() const;

    private:
        Model shipModel = {};
        Color shipColor = {};

        static const int rungCount = 16;
        TrailRung rungs[rungCount];

        float smoothForward = 0;
        float smoothLeft = 0;
        float smoothUp = 0;

        float smoothPitchDown = 0;
        float smoothRollRight = 0;
        float smoothYawLeft = 0;

        float visualBank = 0;

        void positionActiveTrailRung();
        Vector3 lastRungPosition = { 0, 0, 0 };
        int rungIndex = 0;
};

class Crosshair {
    public:
        Crosshair(const char* modelPath);
        ~Crosshair();

        void positionCrosshairOnShip(const Ship& ship, float distance);
        void drawCrosshair() const;

    private:
        Model crosshairModel = {};
};
