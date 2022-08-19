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

#define GAME_TITLE "Hypersonic"

enum class Scene { MAIN_SCENE, GAME_SCENE };

Color textColor = {143, 200, 170, 255};

int g_ScreenWidth = 800;
int g_ScreenHeight = 600;

int g_RenderWidth = 400;
int g_RenderHeight = 300;

void drawStandardFPS() {
    BeginBlendMode(BlendMode::BLEND_ADDITIVE);
    DrawRectangle(5, 5, 45, 15, {143, 200, 170, 100});
    DrawText(TextFormat("FPS %d", GetFPS()), 9, 8, 10, textColor);
    DrawRectangleLinesEx({5, 5, 45, 15}, 0.7, textColor);
    EndBlendMode();
}

void applyInputToShip(Ship& ship) {
    ship.InputForward = 1;

    ship.InputYawLeft = 0;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) ship.InputYawLeft -= 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) ship.InputYawLeft += 1;

    ship.InputYawLeft = Clamp(ship.InputYawLeft, -1, 1);

    ship.InputPitchDown = 0;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) ship.InputPitchDown += 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) ship.InputPitchDown -= 1;

    ship.InputPitchDown = Clamp(ship.InputPitchDown, -1, 1);

    ship.InputRollRight = 0;
    if (IsKeyDown(KEY_Q)) ship.InputRollRight -= 1;
    if (IsKeyDown(KEY_E)) ship.InputRollRight += 1;
}

void printVector3(Vector3 vector) {
    std::cout << "X: " << vector.x << " Y: " << vector.y << " Z: " << vector.z << std::endl;
}

void summonEnemy(Ship &player, Model model, std::vector<Ship> &enemies) {
    Ship other(model, true);
    Vector3 random;
    random.x = GetRandomValue(-10, 10);
    random.y = GetRandomValue(-10, 10);
    random.z = GetRandomValue(-10, 10);
    Vector3 direction = Vector3Normalize(random);
    other.Position = Vector3Add(player.Position, Vector3Scale(direction, 10));
    other.Rotation = player.Rotation;
    enemies.push_back(other);
}

int main() {
    SetConfigFlags(ConfigFlags::FLAG_MSAA_4X_HINT | ConfigFlags::FLAG_VSYNC_HINT | ConfigFlags::FLAG_WINDOW_RESIZABLE);
    InitWindow(g_ScreenWidth, g_ScreenHeight, GAME_TITLE);
    SetExitKey(0);

    // Set up low resolution rendering independent from the window resolution.
    auto renderRatio = (float)g_ScreenWidth / (float)g_RenderWidth;
    RenderTexture2D renderTarget = LoadRenderTexture(g_RenderWidth, g_RenderHeight);
    SetTextureFilter(renderTarget.texture, TextureFilter::TEXTURE_FILTER_POINT);

    // Target height is flipped (in the source rectangle) due to OpenGL reasons.
    Rectangle sourceRect = { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height };
    Rectangle destRect = { -renderRatio, -renderRatio, g_ScreenWidth + (renderRatio * 2), g_ScreenHeight + (renderRatio * 2) };
    Camera2D screenSpaceCamera = { 0 };
    screenSpaceCamera.zoom = 1.0f;

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
    std::vector<Ship> enemies;

    Model shipModel = LoadModel("assets/ship.gltf");

    Ship player(shipModel, false);
    summonEnemy(player, shipModel, enemies);
    Ship other(shipModel, true);
    Vector3 random;
    random.x = GetRandomValue(-10, 10);
    random.y = GetRandomValue(-10, 10);
    random.z = GetRandomValue(-10, 10);
    Vector3 direction = Vector3Normalize(random);
    other.Position = Vector3Add(player.Position, Vector3Scale(direction, 10));
    enemies.push_back(other);

    SpaceDust dust = SpaceDust(25, 255);

    Scene currentScene = Scene::MAIN_SCENE;
    bool gamePaused = false;

    while (!WindowShouldClose()) {
        auto deltaTime = GetFrameTime();

        { // Capture input
            if (!gamePaused) {
                applyInputToShip(player);

                for (auto &enemy : enemies) {
                    applyInputToShip(enemy);
                }
            }

            if (currentScene == Scene::MAIN_SCENE) {
                if (IsKeyPressed(KEY_SPACE)) {
                    currentScene = Scene::GAME_SCENE;
                }
            }

            if (currentScene == Scene::GAME_SCENE) {
                if (IsKeyPressed(KEY_ESCAPE)) {
                    gamePaused = !gamePaused;
                }
            }

            if (IsKeyPressed(KEY_SPACE)) {
                bullets.push_back(Bullet(false, RED, player.Position, Vector3Scale(player.Velocity, 4)));
            }

            if (IsKeyPressed(KEY_I)) {
                summonEnemy(player, shipModel, enemies);
            }
        }

        { // Gameplay updates
            if (!gamePaused) {
                player.Update(deltaTime);

                // Remove dead bullets
                bullets.erase(std::remove_if(bullets.begin(),
                            bullets.end(),
                            [&](Bullet& bullet) {
                            return bullet.isDead;
                            }),
                        bullets.end());

                // Remove dead enemies
                enemies.erase(std::remove_if(enemies.begin(),
                            enemies.end(),
                            [&](Ship& enemy) {
                            return enemy.isDead;
                            }),
                        enemies.end());


                // Update bullets
                for (auto &bullet : bullets) {
                    bullet.updateBullet(deltaTime);

                    for (auto &enemy : enemies) {
                        if (Vector3Distance(enemy.Position, bullet.position) < 0.3) {
                            std::cout << "HIT" << std::endl;
                            bullet.isDead = true;
                            enemy.isDead = true;
                        }
                    }
                }

                // Update enemy
                for (auto &enemy : enemies) {
                    enemy.Update(deltaTime);
                }

                // Position crosshair
                crosshairFar.PositionCrosshairOnShip(player, 40);
                crosshairNear.PositionCrosshairOnShip(player, 20);

                // Camera movement and visual effects
                cameraFlight.FollowShip(player, deltaTime);
                dust.UpdateViewPosition(cameraFlight.GetPosition());
            }
        }

        { // Render in texture
            BeginTextureMode(renderTarget);
            ClearBackground(BLACK);

            {// Start Drawing in 3D
                cameraFlight.Begin3DDrawing();

                {// Draw space background
                    rlDisableBackfaceCulling();
                    rlDisableDepthMask();
                    DrawModel(skybox, cameraFlight.GetPosition(), 5.0f, WHITE);
                    rlEnableBackfaceCulling();
                    rlEnableDepthMask();
                }

                DrawGrid(10, 10);
                player.Draw(false);

                // Draw bullets
                for (auto &bullet : bullets) {
                    bullet.drawBullet();
                }

                // Draw enemies and arrows
                for (auto &enemy : enemies) {
                    enemy.Draw(false);

                    Vector2 enemyPositionOnScreen = GetWorldToScreenEx(enemy.Position,
                            cameraFlight.Camera,
                            g_RenderWidth,
                            g_RenderHeight);

                    Rectangle screenRect;
                    screenRect.width = g_RenderWidth;
                    screenRect.height = g_RenderHeight;

                    if (!CheckCollisionPointRec(enemyPositionOnScreen, screenRect)) {

                        Vector3 pointer = Vector3Subtract(player.Position, enemy.Position);
                        pointer = Vector3Normalize(pointer);
                        Vector3 startPosition = Vector3Add(player.Position, Vector3Scale(pointer, -0.5));
                        Vector3 endPosition = Vector3Add(player.Position, Vector3Scale(pointer, -0.7));
                        DrawCylinderWiresEx(startPosition, endPosition, 0.07, 0, 10, RED);
                    }
                }

                crosshairFar.DrawCrosshair();
                crosshairNear.DrawCrosshair();

                dust.Draw(cameraFlight.GetPosition(), player.Velocity, false);
                cameraFlight.EndDrawing();
            }

            { // UI Code here
                drawStandardFPS();
                if (currentScene == Scene::MAIN_SCENE) {
                    DrawText(GAME_TITLE, g_RenderWidth/2 - (MeasureText(GAME_TITLE, 30)/2), 100, 30, textColor);
                    DrawText("[Press Space]", g_RenderWidth/2 - (MeasureText("[Press Space]", 10)/2), 150, 10, textColor);
                }

                // Draw Pause Screen
                if (gamePaused) {
                    char text[] = "Game Paused";
                    int fontSize = 30;
                    Vector2 measure = MeasureTextEx(GetFontDefault(), text, fontSize, 3);
                    DrawRectangle(0, (g_RenderHeight/2) - 40, g_ScreenWidth, 80, {RED.r, RED.g, RED.b, 100});
                    DrawRectangleLines(-1, (g_RenderHeight/2) - 40, g_ScreenWidth, 80, RED);
                    DrawText(text, g_RenderWidth/2.0 - (measure.x/2.0), g_RenderHeight/2.0 - (measure.y/2.0), 30, RED);
                }
            }
            EndTextureMode();
        }

        {// Draw the render texture target to the screen.
            BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(screenSpaceCamera);

            float scale = MIN((float)GetScreenWidth()/g_RenderWidth, (float)GetScreenHeight()/g_RenderHeight);

            // Draw render texture to screen, properly scaled
            DrawTexturePro(renderTarget.texture, { 0.0f, 0.0f, (float)renderTarget.texture.width, (float)-renderTarget.texture.height },
                    { (GetScreenWidth() - ((float)g_RenderWidth*scale))*0.5f, (GetScreenHeight() - ((float)g_RenderHeight*scale))*0.5f,
                    (float)g_RenderWidth*scale, (float)g_RenderHeight*scale }, { 0, 0 }, 0.0f, WHITE);
            EndMode2D();
            EndDrawing();
        }
    }

    UnloadRenderTexture(renderTarget);
    UnloadModel(shipModel);
    CloseWindow();
    return 0;
}
