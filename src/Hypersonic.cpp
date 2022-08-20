#include "../libs/raylib/src/raylib.h"
#include "../libs/raylib/src/rlgl.h"

#include "Actor.hpp"
#include "Ship.hpp"
#include "SpaceDust.hpp"
#include "GameCamera.hpp"
#include "MathUtils.hpp"
#include "Bullet.hpp"
#include "Asteroid.hpp"
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

int screenWidth = 800;
int screenHeight = 600;

int renderWidth = 400;
int renderHeight = 300;

void drawStandardFPS() {
    BeginBlendMode(BlendMode::BLEND_ADDITIVE);
    DrawRectangle(5, 5, 45, 15, {143, 200, 170, 100});
    DrawText(TextFormat("FPS %d", GetFPS()), 9, 8, 10, textColor);
    DrawRectangleLinesEx({5, 5, 45, 15}, 0.7, textColor);
    EndBlendMode();
}

void applyInputToShip(Ship& ship) {
    ship.inputForward = 1;

    ship.inputYawLeft = 0;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) ship.inputYawLeft -= 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) ship.inputYawLeft += 1;

    ship.inputYawLeft = Clamp(ship.inputYawLeft, -1, 1);

    ship.inputPitchDown = 0;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) ship.inputPitchDown += 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) ship.inputPitchDown -= 1;

    ship.inputPitchDown = Clamp(ship.inputPitchDown, -1, 1);

    ship.inputRollRight = 0;
    if (IsKeyDown(KEY_Q)) ship.inputRollRight -= 1;
    if (IsKeyDown(KEY_E)) ship.inputRollRight += 1;
}

void printVector3(Vector3 vector) {
    std::cout << "X: " << vector.x << " Y: " << vector.y << " Z: " << vector.z << std::endl;
}

void summonEnemy(Ship &player, Model model, std::vector<Ship> &enemies) {
    Ship other(player);
    Vector3 random;
    random.x = GetRandomValue(-10, 10);
    random.y = GetRandomValue(-10, 10);
    random.z = GetRandomValue(-10, 10);
    Vector3 direction = Vector3Normalize(random);
    other.position = Vector3Add(player.position, Vector3Scale(direction, 15));
    other.trailColor = MAROON;
    other.isEnemy = true;
    enemies.push_back(other);
}

void summonAsteroid(Ship &player, std::vector<Asteroid> &asteroids, Model model) {
    Vector3 position = Vector3Add(player.position, Vector3Scale(player.getForward(), 40));
    Vector3 velocity = Vector3Scale(player.getForward(), 20);
    asteroids.push_back(Asteroid(model, position, velocity));
}

bool visibleOnScreen(Vector3 position, Camera camera) {
    Vector2 positionOnScreen = GetWorldToScreenEx(position,
                                                  camera,
                                                  renderWidth,
                                                  renderHeight);

    Rectangle screenRect;
    screenRect.width = renderWidth;
    screenRect.height = renderHeight;

    return CheckCollisionPointRec(positionOnScreen, screenRect);

}

int main() {
    SetConfigFlags(ConfigFlags::FLAG_MSAA_4X_HINT | ConfigFlags::FLAG_VSYNC_HINT | ConfigFlags::FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, GAME_TITLE);
    SetExitKey(0);

    // Set up low resolution rendering independent from the window resolution.
    auto renderRatio = (float)screenWidth / (float)renderWidth;
    RenderTexture2D renderTarget = LoadRenderTexture(renderWidth, renderHeight);
    SetTextureFilter(renderTarget.texture, TextureFilter::TEXTURE_FILTER_POINT);

    // Target height is flipped (in the source rectangle) due to OpenGL reasons.
    Rectangle sourceRect = { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height };
    Rectangle destRect = { -renderRatio, -renderRatio, screenWidth + (renderRatio * 2), screenHeight + (renderRatio * 2) };
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
    std::vector<Asteroid> asteroids;

    Model shipModel = LoadModel("assets/ship.gltf");
    Model asteroidModel = LoadModel("assets/asteroid.gltf");

    Ship player(shipModel, false);
    summonEnemy(player, shipModel, enemies);
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
                bullets.push_back(Bullet(false,
                            RED,
                            player.position,
                            Vector3Scale(player.getForward(),
                                100)));
            }

            if (IsKeyPressed(KEY_I)) {
                summonEnemy(player, shipModel, enemies);
            }

            if (IsKeyPressed(KEY_O)) {
                summonAsteroid(player, asteroids, asteroidModel);
            }
        }

        { // Gameplay updates
            if (!gamePaused) {
                player.update(deltaTime);

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

                // Remove dead asteroids
                asteroids.erase(std::remove_if(asteroids.begin(),
                                asteroids.end(),
                                [&](Asteroid& asteroid) {
                                    return asteroid.isDead;
                                }),
                                asteroids.end());

                // Update bullets
                for (auto &bullet : bullets) {
                    bullet.update(deltaTime);

                    for (auto &enemy : enemies) {
                        if (Vector3Distance(enemy.position, bullet.position) < 0.5) {
                            bullet.isDead = true;
                            enemy.isDead = true;
                        }
                    }
                }

                // Update asteroids
                for (auto &asteroid : asteroids) {
                    asteroid.update(deltaTime);

                    if (Vector3Distance(asteroid.position, player.position) > 50) {
                        asteroid.isDead = true;
                    }
                }

                // Update enemy
                for (auto &enemy : enemies) {
                    enemy.update(deltaTime);
                }

                // Position crosshair
                crosshairFar.positionCrosshairOnShip(player, 40);
                crosshairNear.positionCrosshairOnShip(player, 20);

                // Camera movement and visual effects
                cameraFlight.followShip(player, deltaTime);
                dust.updateViewPosition(cameraFlight.getPosition());
            }
        }

        { // Render in texture
            BeginTextureMode(renderTarget);
            ClearBackground(BLACK);

            {// Start Drawing in 3D
                cameraFlight.begin3DDrawing();

                {// Draw space background
                    rlDisableBackfaceCulling();
                    rlDisableDepthMask();
                    DrawModel(skybox, cameraFlight.getPosition(), 5.0f, WHITE);
                    rlEnableBackfaceCulling();
                    rlEnableDepthMask();
                }

                player.draw(false);

                // Draw bullets
                for (auto &bullet : bullets) {
                    bullet.draw();
                }

                // Draw asteroids
                for (auto &asteroid : asteroids) {
                    asteroid.draw();
                }

                // Draw enemies and arrows
                for (auto &enemy : enemies) {
                    enemy.draw(false);

                    if (!visibleOnScreen(enemy.position, cameraFlight.camera)) {
                        Vector3 pointer = Vector3Subtract(player.position, enemy.position);
                        pointer = Vector3Normalize(pointer);
                        Vector3 startPosition = Vector3Add(player.position, Vector3Scale(pointer, -0.5));
                        Vector3 endPosition = Vector3Add(player.position, Vector3Scale(pointer, -0.7));
                        DrawCylinderWiresEx(startPosition, endPosition, 0.07, 0, 10, RED);
                    }
                }

                crosshairFar.drawCrosshair();
                crosshairNear.drawCrosshair();

                dust.draw(cameraFlight.getPosition(), player.velocity, false);
                cameraFlight.end3DDrawing();
            }

            { // UI Code here
                drawStandardFPS();
                if (currentScene == Scene::MAIN_SCENE) {
                    DrawText(GAME_TITLE, renderWidth/2 - (MeasureText(GAME_TITLE, 30)/2), 100, 30, textColor);
                    DrawText("[Press Space]", renderWidth/2 - (MeasureText("[Press Space]", 10)/2), 150, 10, textColor);
                }

                // Draw Pause Screen
                if (gamePaused) {
                    char text[] = "Game Paused";
                    int fontSize = 30;
                    Vector2 measure = MeasureTextEx(GetFontDefault(), text, fontSize, 3);
                    DrawRectangle(0, (renderHeight/2) - 40, screenWidth, 80, {RED.r, RED.g, RED.b, 100});
                    DrawRectangleLines(-1, (renderHeight/2) - 40, screenWidth, 80, RED);
                    DrawText(text, renderWidth/2.0 - (measure.x/2.0), renderHeight/2.0 - (measure.y/2.0), 30, RED);
                }
            }
            EndTextureMode();
        }

        {// Draw the render texture target to the screen.
            BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(screenSpaceCamera);

            float scale = MIN((float)GetScreenWidth()/renderWidth, (float)GetScreenHeight()/renderHeight);

            // Draw render texture to screen, properly scaled
            DrawTexturePro(renderTarget.texture, { 0.0f, 0.0f, (float)renderTarget.texture.width, (float)-renderTarget.texture.height },
                    { (GetScreenWidth() - ((float)renderWidth*scale))*0.5f, (GetScreenHeight() - ((float)renderHeight*scale))*0.5f,
                    (float)renderWidth*scale, (float)renderHeight*scale }, { 0, 0 }, 0.0f, WHITE);
            EndMode2D();
            EndDrawing();
        }
    }

    UnloadRenderTexture(renderTarget);
    UnloadModel(shipModel);
    UnloadModel(asteroidModel);
    CloseWindow();
    return 0;
}
