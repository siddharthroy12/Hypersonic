#include "Ship.hpp"

#include "MathUtils.hpp"

#include <vector>
#include "../libs/raylib/src/rlgl.h"

static const float RungDistance = 2.0f;
static const float RungTimeToLive = 2.0f;

TrailRung::TrailRung() {}

TrailRung::TrailRung(const TrailRung &oldTrailRung) {
    this->leftPoint = oldTrailRung.leftPoint;
    this->rightPoint = oldTrailRung.rightPoint;
    this->timeToLive = oldTrailRung.timeToLive;
}

Ship::Ship(Model model, bool isEnemy) {
    shipModel = model;
    rotation = QuaternionFromEuler(1, 2, 0);
    shipColor = RAYWHITE;
    lastRungPosition = position;
    this->isEnemy = isEnemy;

    if (isEnemy) {
        this->trailColor = MAROON;
    }
}

Ship::Ship(const Ship &oldShip) {
    this->rotation = oldShip.rotation;
    this->position = oldShip.position;
    this->velocity = oldShip.velocity;
    this->inputForward = oldShip.inputForward;
    this->inputLeft = oldShip.inputLeft;
    this->inputUp = oldShip.inputUp;
    this->inputPitchDown = oldShip.inputPitchDown;
    this->inputRollRight = oldShip.inputRollRight;
    this->inputYawLeft = oldShip.inputYawLeft;

    this->maxSpeed = oldShip.maxSpeed;
    this->throttleResponse = oldShip.throttleResponse;
    this->turnRate = oldShip.turnRate;
    this->turnResponse = oldShip.turnResponse;

    this->length = oldShip.length;
    this->width = oldShip.width;

    this->trailColor = oldShip.trailColor;
    this->isDead = oldShip.isDead;
    this->isEnemy = oldShip.isEnemy;

    this->shipModel = oldShip.shipModel;
    this->shipColor = oldShip.shipColor;

    for (int i = 0; i < this->rungCount; i++) {
        this->rungs[i] = TrailRung(oldShip.rungs[i]);
    }

    this->smoothForward = oldShip.smoothForward;
    this->smoothLeft = oldShip.smoothLeft;
    this->smoothUp = oldShip.smoothUp;

    this->smoothPitchDown = oldShip.smoothPitchDown;
    this->smoothRollRight = oldShip.smoothRollRight;
    this->smoothYawLeft = oldShip.smoothYawLeft;

    this->visualBank = oldShip.visualBank;

    this->lastRungPosition = oldShip.lastRungPosition;
    this->rungIndex = oldShip.rungIndex;
}

void Ship::update(float deltaTime) {
    // Give the ship some momentum when accelerating.
    smoothForward = smoothDamp(smoothForward, inputForward, throttleResponse, deltaTime);
    smoothLeft = smoothDamp(smoothLeft, inputLeft, throttleResponse, deltaTime);
    smoothUp = smoothDamp(smoothUp, inputUp, throttleResponse, deltaTime);

    // Flying in reverse should be slower.
    auto forwardSpeedMultipilier = smoothForward > 0.0f ? 1.0f : 0.33f;

    auto targetVelocity = Vector3Zero();
    targetVelocity = Vector3Add(
            targetVelocity,
            Vector3Scale(getForward(), maxSpeed * forwardSpeedMultipilier * smoothForward));
    targetVelocity = Vector3Add(
            targetVelocity,
            Vector3Scale(getUp(), maxSpeed * .5f * smoothUp));
    targetVelocity = Vector3Add(
            targetVelocity,
            Vector3Scale(getLeft(), maxSpeed * .5f * smoothLeft));

    velocity = smoothDamp(velocity, targetVelocity, 2.5, deltaTime);
    position = Vector3Add(position, Vector3Scale(velocity, deltaTime));

    // Give the ship some inertia when turning. These are the pilot controlled rotations.
    smoothPitchDown = smoothDamp(smoothPitchDown, inputPitchDown, turnResponse, deltaTime);
    smoothRollRight = smoothDamp(smoothRollRight, inputRollRight, turnResponse, deltaTime);
    smoothYawLeft = smoothDamp(smoothYawLeft, inputYawLeft, turnResponse, deltaTime);

    rotateLocalEuler( { 0, 0, 1 }, smoothRollRight * turnRate * deltaTime);
    rotateLocalEuler({ 1, 0, 0 }, smoothPitchDown * turnRate * deltaTime);
    rotateLocalEuler({ 0, 1, 0 }, smoothYawLeft * turnRate * deltaTime);

    //// Auto-roll from yaw
    //// Movement like a 3D space sim. This only feels good if there's no horizon auto-align.
    //RotateLocalEuler({ 0, 0, -1 }, SmoothYawLeft * TurnRate * .5f * deltaTime);

    // Auto-roll to align to horizon
    if (fabs(getForward().y) < 0.8) {
        float autoSteerInput = getRight().y;
        rotateLocalEuler({ 0, 0, 1 }, autoSteerInput * turnRate * .5f * deltaTime);
    }

    // When yawing and strafing, there's some bank added to the model for visual flavor.
    float targetVisualBank = (-30 * DEG2RAD * smoothYawLeft) + (-15 * DEG2RAD * smoothLeft);
    visualBank = smoothDamp(visualBank, targetVisualBank, 10, deltaTime);
    Quaternion visualRotation = QuaternionMultiply(
            rotation, QuaternionFromAxisAngle({ 0, 0, 1 }, visualBank));

    // Sync up the raylib representation of the model with the ship's position so that processing
    // doesn't have to happen at the render stage.
    auto transform = MatrixTranslate(position.x, position.y, position.z);
    transform = MatrixMultiply(QuaternionToMatrix(visualRotation), transform);
    shipModel.transform = transform;

    // The currently active trail rung is dragged directly behind the ship for a smoother trail.
    positionActiveTrailRung();
    if (Vector3Distance(position, lastRungPosition) > RungDistance) {
        rungIndex = (rungIndex + 1) % rungCount;
        lastRungPosition = position;
    }

    for (int i = 0; i < rungCount; ++i)
        rungs[i].timeToLive -= deltaTime;
}

void Ship::positionActiveTrailRung() {
    rungs[rungIndex].timeToLive = RungTimeToLive;
    float halfWidth = width / 2.f;
    float halfLength = length / 2.f;
    rungs[rungIndex].leftPoint = transformPoint({ -halfWidth, 0.0f, -halfLength });
    rungs[rungIndex].rightPoint = transformPoint({ halfWidth, 0.0f, -halfLength });
}

void Ship::draw(bool showDebugAxes) const {
    DrawModel(shipModel, Vector3Zero(), 1, shipColor);

    if (showDebugAxes) {
        BeginBlendMode(BlendMode::BLEND_ADDITIVE);
        DrawLine3D(position, Vector3Add(position, getForward()), { 0, 0, 255, 255 });
        DrawLine3D(position, Vector3Add(position, getLeft()), { 255, 0, 0, 255 });
        DrawLine3D(position, Vector3Add(position, getUp()), { 0, 255, 0, 255 });
        DrawSphereWires(position, 0.3, 5, 5, GRAY);
        EndBlendMode();
    }

    if (this->isEnemy) {
        this->drawTrail();
    }
}

void Ship::drawTrail() const
{
    BeginBlendMode(BlendMode::BLEND_ADDITIVE);
    rlDisableDepthMask();

    for (int i = 0; i < rungCount; ++i)
    {
        if (rungs[i].timeToLive <= 0)
            continue;

        auto& thisRung = rungs[i % rungCount];

        Color color = trailColor;
        color.a = 255 * thisRung.timeToLive / RungTimeToLive;
        Color fill = color;
        fill.a = color.a / 4;

        // The current rung is dragged along behind the ship, so the crossbar shouldn't be drawn.
        // If the crossbar is drawn when the ship is slow, it looks weird having a line behind it.
        if (i != rungIndex)
            DrawLine3D(thisRung.leftPoint, thisRung.rightPoint, color);

        auto& nextRung = rungs[(i + 1) % rungCount];
        if (nextRung.timeToLive > 0 && thisRung.timeToLive < nextRung.timeToLive)
        {
            DrawLine3D(nextRung.leftPoint, thisRung.leftPoint, color);
            DrawLine3D(nextRung.rightPoint, thisRung.rightPoint, color);

            DrawTriangle3D(thisRung.leftPoint, thisRung.rightPoint, nextRung.leftPoint, fill);
            DrawTriangle3D(nextRung.leftPoint, thisRung.rightPoint, nextRung.rightPoint, fill);

            DrawTriangle3D(nextRung.leftPoint, thisRung.rightPoint, thisRung.leftPoint, fill);
            DrawTriangle3D(nextRung.rightPoint, thisRung.rightPoint, nextRung.leftPoint, fill);
        }
    }

    rlDrawRenderBatchActive();
    rlEnableDepthMask();
    EndBlendMode();
}

Crosshair::Crosshair(const char* modelPath)
{
    crosshairModel = LoadModel(modelPath);
}

Crosshair::~Crosshair()
{
    UnloadModel(crosshairModel);
}

void Crosshair::positionCrosshairOnShip(const Ship& ship, float distance)
{
    auto crosshairPos = Vector3Add(Vector3Add(Vector3Scale(ship.getForward(), distance), ship.position), ship.getDown());
    auto crosshairTransform = MatrixTranslate(crosshairPos.x, crosshairPos.y, crosshairPos.z);
    crosshairTransform = MatrixMultiply(QuaternionToMatrix(ship.rotation), crosshairTransform);
    crosshairModel.transform = crosshairTransform;
}

void Crosshair::drawCrosshair() const
{
    BeginBlendMode(BlendMode::BLEND_ADDITIVE);
    rlDisableDepthTest();

    DrawModel(crosshairModel, Vector3Zero(), 1, GREEN);
    //DrawModelWires(CrosshairModel, Vector3Zero(), 1, DARKGREEN);

    rlEnableDepthTest();
    EndBlendMode();
}
