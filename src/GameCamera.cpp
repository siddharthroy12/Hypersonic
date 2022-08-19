#include "GameCamera.hpp"

#include "../libs/raylib/src/raymath.h"

#include "Ship.hpp"
#include "MathUtils.hpp"

GameCamera::GameCamera(bool isPerspective, float fieldOfView) {
    camera = Camera3D();
    camera.position = Vector3{ 0, 10, -10 };
    camera.target = Vector3{ 0, 0, 0 };
    camera.up = Vector3{ 0, 1, 0 };

    camera.fovy = fieldOfView;
    camera.projection = isPerspective
        ? CameraProjection::CAMERA_PERSPECTIVE
        : CameraProjection::CAMERA_ORTHOGRAPHIC;

    smoothPosition = Vector3Zero();
    smoothTarget = Vector3Zero();
    smoothUp = Vector3Zero();
}

void GameCamera::followShip(const Ship& ship, float deltaTime) {
    Vector3 position = ship.transformPoint({ 0, 1, -1 });
    Vector3 shipForwards = Vector3Scale(ship.getForward(), 25);
    Vector3 target = Vector3Add(ship.position, shipForwards);
    Vector3 up = ship.getUp();

    moveTo(position, target, up, deltaTime);
}

void GameCamera::moveTo(Vector3 position, Vector3 target, Vector3 up, float deltaTime) {
    camera.position = smoothDamp(
            camera.position, position,
            20, deltaTime);

    camera.target = smoothDamp(
            camera.target, target,
            5, deltaTime);

    camera.up = smoothDamp(
            camera.up, up,
            5, deltaTime);
}

void GameCamera::setPosition(Vector3 position, Vector3 target, Vector3 up)
{
    camera.position = position;
    camera.target = target;
    camera.up = up;

    smoothPosition = position;
    smoothTarget = target;
    smoothUp = up;
}

Vector3 GameCamera::getPosition() const {
    return camera.position;
}

void GameCamera::begin3DDrawing() const {
    BeginMode3D(camera);
}

void GameCamera::end3DDrawing() const {
    EndMode3D();
}
