#pragma once

#include "../libs/raylib/src/raylib.h"

class Ship;

class GameCamera {
    public:

        GameCamera(bool isPerspective, float fieldOfView);

        // Automatically moves the camera to follow a target ship.
        void followShip(const Ship& ship, float deltaTime);

        // Moves the camera to the given positions. Smoothing is automatically applied.
        void moveTo(Vector3 position, Vector3 target, Vector3 up, float deltaTime);

        // Immediately moves the camera to the given positions with no smoothing.
        void setPosition(Vector3 position, Vector3 target, Vector3 up);

        // Required to tell raylib that any further 3D calls will be made with this camera.
        // Must be paired with EndDrawing().
        void begin3DDrawing() const;

        // Requires to tell raylib to stop 3D rendering with this camera.
        void end3DDrawing() const;

        Vector3 getPosition() const;

        Camera3D camera;
    private:
        Vector3 smoothPosition;
        Vector3 smoothTarget;
        Vector3 smoothUp;
};
