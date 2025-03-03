#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include "constants.h"

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct Player
{
	float x;
	float y;
	float width;
	float height;
	int turnDirection; // -1 for left, +1 for right
	int walkDirection; // -1 for back, +1 for front
	float rotationAngle;
	float walkSpeed;
	float turnSpeed;
} player;

struct MyRay
{
	float rayAngle;
	float wallHitX;
	float wallHitY;
	float distance;
	int wasHitVertical;
	int isRayFacingUp;
	int isRayFacingDown;
	int isRayFacingLeft;
	int isRayFacingRight;
	int wallHitContent; // fixme: don't like this, the map should be more complex than this
} rays[NUM_RAYS];

struct ColorBuffer
{
	uint32_t *Buffer;
	Texture2D texture;
} colorBuffer;

//--------------------------------------------------------------------------------------
// Update functions
//--------------------------------------------------------------------------------------

bool isWallAt(float x, float y)
{
	int mapGridIndexX = floor(x / TILE_SIZE);
	int mapGridIndexY = floor(y / TILE_SIZE);

	bool isWall = map[mapGridIndexY][mapGridIndexX] == 1;
	return isWall;
}

bool isOutOfBoundaries(float x, float y)
{
	return x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT;
}

void movePlayer(float deltaTime)
{
	player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
	float moveStep = player.walkDirection * player.walkSpeed * deltaTime;
	float newPlayerX = player.x + cos(player.rotationAngle) * moveStep;
	float newPlayerY = player.y + sin(player.rotationAngle) * moveStep;

	if (!isWallAt(newPlayerX, newPlayerY) && !isOutOfBoundaries(newPlayerX, newPlayerY))
	{
		player.x = newPlayerX;
		player.y = newPlayerY;
	}
}

float normalizeAngle(float rayAngle)
{
	rayAngle = remainder(rayAngle, TWO_PI);
	if (rayAngle < 0)
	{
		rayAngle += TWO_PI;
	}
	return rayAngle;
}

float distanceBetweenTwoPoints(float x1, float y1, float x2, float y2)
{
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void castRay(float rayAngle, int rayIndex)
{
	rayAngle = normalizeAngle(rayAngle);

	int isRayFacingDown = rayAngle > 0 && rayAngle < PI;
	int isRayFacingUp = !isRayFacingDown;
	int isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
	int isRayFacingLeft = !isRayFacingRight;

	float xintercept, yintercept;
	float xstep, ystep;

	int foundHorizontalWallHit = FALSE;
	float horizontalWallHitX = 0;
	float horizontalWallHitY = 0;

	yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
	yintercept += isRayFacingDown ? TILE_SIZE : 0;

	xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

	ystep = TILE_SIZE;
	ystep *= isRayFacingUp ? -1 : 1;
	xstep = TILE_SIZE / tan(rayAngle);
	xstep *= isRayFacingLeft && xstep > 0 ? -1 : 1;
	xstep *= isRayFacingRight && xstep < 0 ? -1 : 1;

	float nextHorizontalIntersectionX = xintercept;
	float nextHorizontalIntersectionY = yintercept;

	while (nextHorizontalIntersectionX >= 0 && nextHorizontalIntersectionX <= WINDOW_WIDTH && nextHorizontalIntersectionY >= 0 && nextHorizontalIntersectionY <= WINDOW_HEIGHT)
	{
		if (isWallAt(nextHorizontalIntersectionX, nextHorizontalIntersectionY - (isRayFacingUp ? 1 : 0)))
		{
			foundHorizontalWallHit = TRUE;
			horizontalWallHitX = nextHorizontalIntersectionX;
			horizontalWallHitY = nextHorizontalIntersectionY;
			// TODO: store grid colors and other infos
			break;
		}
		else
		{
			nextHorizontalIntersectionX += xstep;
			nextHorizontalIntersectionY += ystep;
		}
	}

	// Compute vertical intersection with the grid
	int foundVerticalWallHit = FALSE;
	float verticalWallHitX = 0;
	float verticalWallHitY = 0;

	xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
	xintercept += isRayFacingRight ? TILE_SIZE : 0;
	yintercept = player.y + (xintercept - player.x) * tan(rayAngle);

	// Compute steps
	xstep = TILE_SIZE;
	xstep *= isRayFacingLeft ? -1 : 1;
	ystep = TILE_SIZE * tan(rayAngle);
	ystep *= isRayFacingUp && ystep > 0 ? -1 : 1;
	ystep *= isRayFacingDown && ystep < 0 ? -1 : 1;

	float nextVerticalIntersectionX = xintercept;
	float nextVerticalIntersectionY = yintercept;

	while (nextVerticalIntersectionX >= 0 && nextVerticalIntersectionX <= WINDOW_WIDTH && nextVerticalIntersectionY >= 0 && nextVerticalIntersectionY <= WINDOW_HEIGHT)
	{

		if (isWallAt(nextVerticalIntersectionX - (isRayFacingLeft ? 1 : 0), nextVerticalIntersectionY))
		{
			foundVerticalWallHit = TRUE;
			verticalWallHitX = nextVerticalIntersectionX;
			verticalWallHitY = nextVerticalIntersectionY;
			break;
		}
		else
		{
			nextVerticalIntersectionX += xstep;
			nextVerticalIntersectionY += ystep;
		}
	}

	float horizontalIntersectionDistance = foundHorizontalWallHit ? distanceBetweenTwoPoints(player.x, player.y, horizontalWallHitX, horizontalWallHitY) : INT_MAX;
	float verticalIntersectionDistance = foundVerticalWallHit ? distanceBetweenTwoPoints(player.x, player.y, verticalWallHitX, verticalWallHitY) : INT_MAX;

	float wallHitX = horizontalIntersectionDistance < verticalIntersectionDistance ? horizontalWallHitX : verticalWallHitX;
	float wallHitY = horizontalIntersectionDistance < verticalIntersectionDistance ? horizontalWallHitY : verticalWallHitY;
	float distance = horizontalIntersectionDistance < verticalIntersectionDistance ? horizontalIntersectionDistance : verticalIntersectionDistance;
	int wasHitVertical = verticalIntersectionDistance < horizontalIntersectionDistance;

	rays[rayIndex].distance = distance;
	rays[rayIndex].wallHitX = wallHitX;
	rays[rayIndex].wallHitY = wallHitY;
	// rays[rayIndex].tile = tile;
	rays[rayIndex].wasHitVertical = wasHitVertical;
	rays[rayIndex].rayAngle = rayAngle;
	rays[rayIndex].isRayFacingDown = isRayFacingDown;
	rays[rayIndex].isRayFacingUp = isRayFacingUp;
	rays[rayIndex].isRayFacingRight = isRayFacingRight;
	rays[rayIndex].isRayFacingLeft = isRayFacingLeft;
}

void castRays()
{
	float rayAngle = player.rotationAngle - (FOV_ANGLE / 2);
	for (int i = 0; i < NUM_RAYS; i++)
	{
		castRay(rayAngle, i);
		rayAngle += FOV_ANGLE / NUM_RAYS;
	}
}

//--------------------------------------------------------------------------------------
// Rendering functions
//--------------------------------------------------------------------------------------

void generate3DWallProjection()
{
	for (int i = 0; i < NUM_RAYS; i++)
	{
		float normalizedDistance = rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);
		float distanceFromProjectionPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
		float projectedWallHeight = (TILE_SIZE / normalizedDistance) * distanceFromProjectionPlane;

		int wallStripHeight = projectedWallHeight;
		int wallStartPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
		wallStartPixel = wallStartPixel < 0 ? 0 : wallStartPixel;
		int wallEndPixel = wallStartPixel + wallStripHeight;
		wallEndPixel = wallEndPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallEndPixel;

		for (int y = 0; y < wallStartPixel; y++) 
		{
			colorBuffer.Buffer[WINDOW_WIDTH * y + i] = 0xFF333333;
		}

		for (int y = wallStartPixel; y < wallEndPixel; y++)
		{
			colorBuffer.Buffer[WINDOW_WIDTH * y + i] = rays[i].wasHitVertical ? 0xFFFFFFFF : 0xFFCCCCCC;
		}

		for (int y = wallEndPixel; y < WINDOW_HEIGHT; y++) 
		{
			colorBuffer.Buffer[WINDOW_WIDTH * y + i] = 0xFF777777;
		}
	}
}

void clearColorBuffer(uint32_t color)
{
	for (int x = 0; x < WINDOW_WIDTH; x++)
	{
		for (int y = 0; y < WINDOW_HEIGHT; y++)
		{
			colorBuffer.Buffer[WINDOW_WIDTH * y + x] = color;
		}
	}
}

void renderColorBuffer()
{
	UpdateTexture(colorBuffer.texture, colorBuffer.Buffer);
	DrawTexture(colorBuffer.texture, 0, 0, WHITE);
}

void renderMap()
{
	for (int i = 0; i < MAP_NUM_ROWS; i++)
	{
		for (int j = 0; j < MAP_NUM_COLS; j++)
		{
			int tileX = j * TILE_SIZE;
			int tileY = i * TILE_SIZE;
			Color tileColor = map[i][j] != 0 ? WHITE : BLACK;

			DrawRectangle(
					tileX * MINIMAP_SCALE_FACTOR,
					tileY * MINIMAP_SCALE_FACTOR,
					TILE_SIZE * MINIMAP_SCALE_FACTOR,
					TILE_SIZE * MINIMAP_SCALE_FACTOR,
					tileColor);
		}
	}
}
void renderPlayer()
{
	DrawRectangle(
			player.x * MINIMAP_SCALE_FACTOR,
			player.y * MINIMAP_SCALE_FACTOR,
			player.width * MINIMAP_SCALE_FACTOR,
			player.height * MINIMAP_SCALE_FACTOR,
			RED);
	DrawLine(
			player.x * MINIMAP_SCALE_FACTOR,
			player.y * MINIMAP_SCALE_FACTOR,
			MINIMAP_SCALE_FACTOR * (player.x + cos(player.rotationAngle) * 40),
			MINIMAP_SCALE_FACTOR * (player.y + sin(player.rotationAngle) * 40),
			RED);
}

void renderRays()
{
	for (int i = 0; i < NUM_RAYS; i++)
	{
		struct MyRay ray = rays[i];
		float wallDistance = ray.distance * cos(ray.rayAngle - player.rotationAngle);
		float projectionPlaneDistance = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE);
		float wallStripHeight = (TILE_SIZE / wallDistance) * projectionPlaneDistance;

		float alpha = 150 / wallDistance;
		float color = ray.wasHitVertical ? 255 : 200;

		DrawLine(
				MINIMAP_SCALE_FACTOR * player.x,
				MINIMAP_SCALE_FACTOR * player.y,
				MINIMAP_SCALE_FACTOR * ray.wallHitX,
				MINIMAP_SCALE_FACTOR * ray.wallHitY,
				RED);
	}
}

//--------------------------------------------------------------------------------------
// Game loop functions
//--------------------------------------------------------------------------------------

void setup()
{
	player.x = WINDOW_WIDTH / 2;
	player.y = WINDOW_HEIGHT / 2;
	player.width = 1;
	player.height = 1;
	player.turnDirection = 0;
	player.walkDirection = 0;
	player.rotationAngle = PI / 2;
	player.walkSpeed = 100;
	player.turnSpeed = 45 * (PI / 180);

	// colorBuffer = (u_int32_t *)malloc(sizeof(u_int32_t) * (u_int32_t)WINDOW_WIDTH * (u_int32_t)WINDOW_HEIGHT);
	Image image = {
			.data = NULL,
			.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
			.mipmaps = 1,
			.height = WINDOW_HEIGHT,
			.width = WINDOW_WIDTH};

	colorBuffer.Buffer = (u_int32_t *)malloc(sizeof(u_int32_t) * (u_int32_t)WINDOW_WIDTH * (u_int32_t)WINDOW_HEIGHT);
	colorBuffer.texture = LoadTextureFromImage(image);
}

void processInput()
{
	player.turnDirection = 0;
	player.walkDirection = 0;

	if (IsKeyDown(KEY_RIGHT))
		player.turnDirection = +1;
	if (IsKeyDown(KEY_LEFT))
		player.turnDirection = -1;
	if (IsKeyDown(KEY_UP))
		player.walkDirection = +1;
	if (IsKeyDown(KEY_DOWN))
		player.walkDirection = -1;
}

void update(float deltaTime)
{
	movePlayer(deltaTime);
	castRays();
}

void render()
{
	BeginDrawing();
	ClearBackground(BLACK);

	generate3DWallProjection();

	renderColorBuffer();
	clearColorBuffer(0xFF000000);

	renderMap();
	renderPlayer();
	renderRays();

	EndDrawing();
}

//--------------------------------------------------------------------------------------
// MAIN
//--------------------------------------------------------------------------------------
int main()
{
	// Initialization
	//--------------------------------------------------------------------------------------
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Raycaster");
	SearchAndSetResourceDir("resources");
	SetTargetFPS(60);
	setup();
	//--------------------------------------------------------------------------------------

	// game loop
	while (!WindowShouldClose())
	{
		float deltaTime = GetFrameTime();

		// Process Input
		//--------------------------------------------------------------------------------------
		processInput();
		//--------------------------------------------------------------------------------------

		// Update
		//--------------------------------------------------------------------------------------
		update(deltaTime);
		//--------------------------------------------------------------------------------------

		// Render
		//--------------------------------------------------------------------------------------
		render();
		//--------------------------------------------------------------------------------------
	}

	UnloadTexture(colorBuffer.texture);
	free(colorBuffer.Buffer);

	CloseWindow();
	return 0;
}
