#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>

// TILE DIMENSIONS
#define TILE_SIZE     16   // Each tile is 16x16 pixels

// LEVEL DIMENSIONS (in tiles)
#define LEVEL_WIDTH   80   // 80 tiles wide = 1280 pixels (scrolls horizontally)
#define LEVEL_HEIGHT  15   // 15 tiles tall = 240 pixels (fits screen exactly)

// TILE TYPE DEFINITIONS
#define TILE_AIR          0   // Empty space - not solid
#define TILE_GROUND       1   // Solid ground block
#define TILE_BRICK        2   // Brick block
#define TILE_SPIKE        3   // Spike trap - kills player when hit
#define TILE_FAKE_GROUND  4   // Looks like ground but collapses - TRAP
#define TILE_HIDDEN_SPIKE 5   // Invisible until player is near - TRAP
#define TILE_GOAL         6   // Goal flag

// TILE COLOUR INDICES (colours corresponds to LCD's colour)
#define COLOUR_GROUND       3
#define COLOUR_BRICK        2
#define COLOUR_SPIKE        4
#define COLOUR_FAKE_GROUND  3
#define COLOUR_GOAL         5

// Initialize the level by copying data from flash to RAM
void Level_Init(void);

// Get the tile type at a given position
uint8_t Level_GetTile(int16_t col, int16_t row);

// Set tile to a new type (for breaking blocks)
void Level_SetTile(int16_t col, int16_t row, uint8_t tile);

// Check if a tile is solid (to block player's movement)
uint8_t Level_IsSolid(int16_t col, int16_t row);

// Draw all tiles to the LCD buffer
void Level_Draw(int16_t camera_x);

// Collapse fake ground tiles
void Level_BreakFakeGround(void);

// Reveal hidden spikes near the player. Get x and y position from player 
void Level_RevealTraps(float player_x, float player_y);

#endif