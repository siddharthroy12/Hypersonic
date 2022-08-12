#include "raylib.h"

#include "Actor.h"
#include "Ship.h"
#include "SpaceDust.h"
#include "GameCamera.h"
#include "MathUtils.h"

//#define RENDER_SMALL

int g_ScreenWidth = 800;
int g_ScreenHeight = 600;

#ifdef RENDER_SMALL
int g_RenderWidth = 400;
int g_RenderHeight = 300;
#endif // RENDER_SMALL

void DrawStandardFPS()
{
	BeginBlendMode(BlendMode::BLEND_ADDITIVE);
	DrawText(TextFormat("FPS %d", GetFPS()), 10, 10, 10, GREEN);
	EndBlendMode();
}

void ApplyInputToShip(Ship& ship)
{
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

	ship.InputUp = 0;
	if (IsKeyDown(KEY_SPACE)) ship.InputUp += 1;
	if (IsKeyDown(KEY_LEFT_CONTROL)) ship.InputUp -= 1;

	auto triggerRight = GetGamepadAxisMovement(0, GamepadAxis::GAMEPAD_AXIS_RIGHT_TRIGGER);
	triggerRight = Remap(triggerRight, -1, 1, 0, 1);

	auto triggerLeft = GetGamepadAxisMovement(0, GamepadAxis::GAMEPAD_AXIS_LEFT_TRIGGER);
	triggerLeft = Remap(triggerLeft, -1, 1, 0, 1);

	ship.InputUp += triggerRight;
	ship.InputUp -= triggerLeft;
	ship.InputUp = Clamp(ship.InputUp, -1, 1);

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

int main()
{
	SetConfigFlags(ConfigFlags::FLAG_MSAA_4X_HINT | ConfigFlags::FLAG_VSYNC_HINT);
	InitWindow(g_ScreenWidth, g_ScreenHeight, "Ergo");

#ifdef RENDER_SMALL
	// Set up low resolution rendering independent from the window resolution.
	auto renderRatio = (float)g_ScreenWidth / (float)g_RenderWidth;
	RenderTexture2D renderTarget = LoadRenderTexture(g_RenderWidth, g_RenderHeight);
	SetTextureFilter(renderTarget.texture, TextureFilter::TEXTURE_FILTER_POINT);


	// Target height is flipped (in the source rectangle) due to OpenGL reasons.
	Rectangle sourceRect = { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height };
	Rectangle destRect = { -renderRatio, -renderRatio, g_ScreenWidth + (renderRatio * 2), g_ScreenHeight + (renderRatio * 2) };
	Camera2D screenSpaceCamera = { 0 };
	screenSpaceCamera.zoom = 1.0f;
#endif // RENDER_SMALL

	Ship player("assets/ship.gltf", "assets/a16.png", RAYWHITE);
	Ship other("assets/ship.gltf", "assets/a16.png", RAYWHITE);
	other.TrailColor = MAROON;
	other.Position = { 10, 2, 10 };
	SpaceDust dust = SpaceDust(25, 255);

	GameCamera cameraFlight = GameCamera(true, 50);
	GameCamera cameraHUD = GameCamera(false, 50);
	cameraHUD.SetPosition({ 0, 0, -10 }, { 0, 0, 0 }, { 0, 1, 0 });

	// Test station.
	Texture2D texture = LoadTexture("assets/a16.png");
	texture.mipmaps = 0;
	SetTextureFilter(texture, TEXTURE_FILTER_POINT);
	Model stationModel = LoadModel("assets/station.gltf");
	stationModel.materials[0].maps[MaterialMapIndex::MATERIAL_MAP_ALBEDO].texture = texture;
	stationModel.transform = MatrixTranslate(0, 5, 50);

	Crosshair crosshairNear = Crosshair("assets/crosshair2.gltf");
	Crosshair crosshairFar = Crosshair("assets/crosshair2.gltf");

	while (!WindowShouldClose())
	{
		auto deltaTime = GetFrameTime();

		// Capture input
		{
			ApplyInputToShip(player);
			ApplyInputToShip(other);
		}

		// Gameplay updates
		{
			player.Update(deltaTime);
			other.Update(deltaTime);

			crosshairNear.PositionCrosshairOnShip(player, 10);
			crosshairFar.PositionCrosshairOnShip(player, 30);
		}

		// Camera movement and visual effects
		{
			cameraFlight.FollowShip(player, deltaTime);
			dust.UpdateViewPosition(cameraFlight.GetPosition());
		}

		// Render
#ifdef RENDER_SMALL
		BeginTextureMode(renderTarget);
#else
		BeginDrawing();
#endif
		{
			ClearBackground({32, 32, 64, 255});

			cameraFlight.Begin3DDrawing();
			{
				// Opaques
				{
					DrawGrid(10, 10);

					player.Draw(false);
					other.Draw(false);

					DrawModel(stationModel, Vector3Zero(), 1, WHITE);
				}

				// Transparencies
				{
					player.DrawTrail();
					other.DrawTrail();

					crosshairNear.DrawCrosshair();
					crosshairFar.DrawCrosshair();

					dust.Draw(cameraFlight.GetPosition(), player.Velocity, false);
				}
			}
			cameraFlight.EndDrawing();

			//cameraHUD.Begin3DDrawing();
			//{
			//	// Test for drawing 3D geometry to an orthrographic "HUD".
			//	DrawSphereWires({sinf(GetTime()) * 12}, 3, 8, 8, RED);
			//}
			//cameraHUD.EndDrawing();

			DrawStandardFPS();
		}
#ifdef RENDER_SMALL
		EndTextureMode();
#else
		EndDrawing();
#endif

#ifdef RENDER_SMALL
		// Draw the render texture target to the screen.
		BeginDrawing();
		{
			ClearBackground(RED);

			BeginMode2D(screenSpaceCamera);
			DrawTexturePro(renderTarget.texture, sourceRect, destRect, { 0, 0 }, 0, WHITE);
			EndMode2D();
		}
		EndDrawing();
#endif // RENDER_SMALL
	}

	CloseWindow();

	return 0;
}
