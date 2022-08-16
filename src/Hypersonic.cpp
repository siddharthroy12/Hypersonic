#include "../libs/raylib/src/raylib.h"
#include "../libs/raylib/src/rlgl.h"

#include "Actor.h"
#include "Ship.h"
#include "SpaceDust.h"
#include "GameCamera.h"
#include "MathUtils.h"
#include "Bullet.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

enum class Scene { MAIN_SCENE, GAME_SCENE };

int g_ScreenWidth = 800;
int g_ScreenHeight = 600;

int g_RenderWidth = 400;
int g_RenderHeight = 300;

void drawStandardFPS() {
    BeginBlendMode(BlendMode::BLEND_ADDITIVE);
    DrawText(TextFormat("FPS %d", GetFPS()), 10, 10, 10, Color{143, 200, 170, 255});
    EndBlendMode();
}

void applyInputToShip(Ship& ship, std::vector<Bullet>& bullets) {
    if (IsKeyPressed(KEY_SPACE)) {
        bullets.push_back(Bullet(RED, ship.Position, ship.GetForward()));
    }

    ship.InputForward = 0;
    if (IsKeyDown(KEY_W)) ship.InputForward += 1;
    if (IsKeyDown(KEY_S)) ship.InputForward -= 1;

    ship.InputForward -= GetGamepadAxisMovement(0, GamepadAxis::GAMEPAD_AXIS_LEFT_Y);
    ship.InputForward = Clamp(ship.InputForward, -1, 1);

    ship.InputLeft = 0;
    if (IsKeyDown(KEY_D)) ship.InputLeft -= 1;
    if (IsKeyDown(KEY_A)) ship.InputLeft += 1;

    ship.InputLeft -= GetGamepadAxisMovement(0, GamepadAxis::GAMEPAD_AXIS_LEFT_X);
    ship.InputLeft = Clamp(ship.InputLeft, -1, 1);

    ship.InputYawLeft = 0;
    if (IsKeyDown(KEY_RIGHT)) ship.InputYawLeft -= 1;
    if (IsKeyDown(KEY_LEFT)) ship.InputYawLeft += 1;

    ship.InputYawLeft -= GetGamepadAxisMovement(0, GamepadAxis::GAMEPAD_AXIS_RIGHT_X);
    ship.InputYawLeft = Clamp(ship.InputYawLeft, -1, 1);

    ship.InputPitchDown = 0;
    if (IsKeyDown(KEY_UP)) ship.InputPitchDown += 1;
    if (IsKeyDown(KEY_DOWN)) ship.InputPitchDown -= 1;

    ship.InputPitchDown += GetGamepadAxisMovement(0, GamepadAxis::GAMEPAD_AXIS_RIGHT_Y);
    ship.InputPitchDown = Clamp(ship.InputPitchDown, -1, 1);

    ship.InputRollRight = 0;
    if (IsKeyDown(KEY_Q)) ship.InputRollRight -= 1;
    if (IsKeyDown(KEY_E)) ship.InputRollRight += 1;
}

int main() {
    SetConfigFlags(ConfigFlags::FLAG_MSAA_4X_HINT | ConfigFlags::FLAG_VSYNC_HINT | ConfigFlags::FLAG_WINDOW_RESIZABLE);
    InitWindow(g_ScreenWidth, g_ScreenHeight, "Hypersonic");

    // Set up low resolution rendering independent from the window resolution.
    auto renderRatio = (float)g_ScreenWidth / (float)g_RenderWidth;
    RenderTexture2D renderTarget = LoadRenderTexture(g_RenderWidth, g_RenderHeight);
    SetTextureFilter(renderTarget.texture, TextureFilter::TEXTURE_FILTER_POINT);

    // Target height is flipped (in the source rectangle) due to OpenGL reasons.
    Rectangle sourceRect = { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height };
    Rectangle destRect = { -renderRatio, -renderRatio, g_ScreenWidth + (renderRatio * 2), g_ScreenHeight + (renderRatio * 2) };
    Camera2D screenSpaceCamera = { 0 };
    screenSpaceCamera.zoom = 1.0f;

    Ship player("assets/ship.gltf", "assets/a16.png", RAYWHITE);
    Ship other("assets/ship.gltf", "assets/a16.png", RAYWHITE);
    other.TrailColor = MAROON;
    other.Position = { 10, 2, 10 };
    SpaceDust dust = SpaceDust(25, 255);


    // Load skybox model
    Mesh cube = GenMeshCube(10.0f, 10.0f, 10.0f);
    Model skybox = LoadModelFromMesh(cube);

    // Background Image
    Texture2D backgroundTexture = LoadTexture("assets/background.png");

    SetMaterialTexture(&skybox.materials[0], MATERIAL_MAP_DIFFUSE, backgroundTexture);

    // Camera
    GameCamera cameraFlight = GameCamera(true, 50);

    Crosshair crosshairFar = Crosshair("assets/crosshairNew.gltf");
    Crosshair crosshairNear = Crosshair("assets/crosshairNew.gltf");

    std::vector<Bullet> bullets;

    while (!WindowShouldClose()) {
        auto deltaTime = GetFrameTime();

        // Capture input
        applyInputToShip(player, bullets);
        applyInputToShip(other, bullets);

        // Gameplay updates
        player.Update(deltaTime);
        other.Update(deltaTime);

        // Remove dead bullets
        bullets.erase(std::remove_if(bullets.begin(),
                      bullets.end(),
                      [&](Bullet& bullet) {
                            return bullet.isDead;
                      }),
                      bullets.end());


        // Position crosshair
        crosshairFar.PositionCrosshairOnShip(player, 40);
        crosshairNear.PositionCrosshairOnShip(player, 20);

        // Camera movement and visual effects
        cameraFlight.FollowShip(player, deltaTime);
        dust.UpdateViewPosition(cameraFlight.GetPosition());

        // Render in texture
        BeginTextureMode(renderTarget);
        ClearBackground(BLACK);

        // Start Drawing in 3D
        cameraFlight.Begin3DDrawing();


        // Draw space background
        rlDisableBackfaceCulling();
        rlDisableDepthMask();
        DrawModel(skybox, cameraFlight.GetPosition(), 5.0f, WHITE);
        rlEnableBackfaceCulling();
        rlEnableDepthMask();


        DrawGrid(10, 10);
        player.Draw(false);
        other.Draw(false);


        // Draw and update bullets
        for (auto &bullet : bullets) {
            bullet.updateBullet(deltaTime);
            bullet.drawBullet();
        }

        player.DrawTrail();
        other.DrawTrail();

        crosshairFar.DrawCrosshair();
        crosshairNear.DrawCrosshair();

        dust.Draw(cameraFlight.GetPosition(), player.Velocity, false);
        cameraFlight.EndDrawing();

        drawStandardFPS();
        EndTextureMode();

        // Draw the render texture target to the screen.
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(screenSpaceCamera);

        float scale = MIN((float)GetScreenWidth()/g_RenderWidth, (float)GetScreenHeight()/g_RenderHeight);

        // Draw render texture to screen, properly scaled
        DrawTexturePro(renderTarget.texture, (Rectangle){ 0.0f, 0.0f, (float)renderTarget.texture.width, (float)-renderTarget.texture.height },
                (Rectangle){ (GetScreenWidth() - ((float)g_RenderWidth*scale))*0.5f, (GetScreenHeight() - ((float)g_RenderHeight*scale))*0.5f,
                (float)g_RenderWidth*scale, (float)g_RenderHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndMode2D();
        EndDrawing();
    }

    UnloadRenderTexture(renderTarget);
    CloseWindow();
    return 0;
}
