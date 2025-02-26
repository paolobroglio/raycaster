#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include "constants.h"

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 1},
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
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct Player {
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

void setup() {
	player.x = WINDOW_WIDTH / 2;
	player.y = WINDOW_HEIGHT / 2;
	player.width = 5;
	player.height = 5;
	player.turnDirection = 0;
	player.walkDirection = 0;
	player.rotationAngle = PI / 2;
	player.walkSpeed = 100;
	player.turnSpeed = 45 * (PI / 180);
}

void renderMap() {
	for (int i = 0; i < MAP_NUM_ROWS; i++) {
			for (int j = 0; j < MAP_NUM_COLS; j++) {
					int tileX = j * TILE_SIZE;
					int tileY = i * TILE_SIZE;
					Color tileColor = map[i][j] != 0 ? WHITE : BLACK;
					

					DrawRectangle(
						tileX * MINIMAP_SCALE_FACTOR,
						tileY * MINIMAP_SCALE_FACTOR,
						TILE_SIZE * MINIMAP_SCALE_FACTOR,
						TILE_SIZE * MINIMAP_SCALE_FACTOR,
						tileColor
					);
			}
	}
}
void renderPlayer(){}



int main ()
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
		// Update
		//--------------------------------------------------------------------------------------
		
		//--------------------------------------------------------------------------------------
	
		
		// Render
  	//--------------------------------------------------------------------------------------
		BeginDrawing();
		ClearBackground(BLACK);
		
		renderMap();
		renderPlayer();
		
		EndDrawing();
		//--------------------------------------------------------------------------------------
	}

	CloseWindow();
	return 0;
}
