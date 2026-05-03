#include "Level.h"
#include "LCD.h"
#include <string.h>

// SCREEN DIMENSIONS
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

static uint8_t level_map[LEVEL_HEIGHT][LEVEL_WIDTH];

// Tile numbers:
// 0 = Air       1 = Ground    2 = Brick     3 = Spike
// 4 = FakeGnd   5 = HidSpike  6 = Goal
static const uint8_t level_data[LEVEL_HEIGHT][LEVEL_WIDTH] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Row 0
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Row 1
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Row 2
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Row 3
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Row 4
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, // Row 5
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 1, 0, 0, 0, 0, 0}, // Row 6
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0}, // Row 7
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Row 8
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 5, 0}, // Row 9
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 4, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 5, 6}, // Row 10
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1}, // Row 11
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}, // Row 12
    {1, 1, 1, 1, 1, 4, 4, 0, 0, 1, 1, 1, 1, 1, 1, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 1, 1, 1, 1}, // Row 13
    {1, 1, 1, 1, 1, 4, 4, 3, 3, 1, 1, 1, 1, 1, 1, 4, 4, 0, 0, 0, 0, 3, 3, 3, 3, 1, 1, 1, 3, 3, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 3, 3, 0, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Row 14
};

// Initialize the level by copying data from flash to RAM
void Level_Init(void){
    memcpy(level_map, level_data, sizeof(level_data));
}

// Get the tile type at a given position
uint8_t Level_GetTile(int16_t col, int16_t row){
    if(col < 0 || col >= LEVEL_WIDTH || row < 0 || row >= LEVEL_HEIGHT){
        return TILE_AIR;
    }
    return level_map[row][col];
}

// Set tile to a new type (for breaking blocks)
void Level_SetTile(int16_t col, int16_t row, uint8_t tile){
    if(col >= 0 && col < LEVEL_WIDTH && row >= 0 && row < LEVEL_HEIGHT){
        level_map[row][col] = tile;
    }
}

// Check if a tile is solid (to block player's movement)
uint8_t Level_IsSolid(int16_t col, int16_t row){
    uint8_t tile = Level_GetTile(col, row);
    switch(tile){
        case TILE_GROUND:
        case TILE_BRICK:
        case TILE_FAKE_GROUND:
            return 1;
        default:
            return 0;
    }
}

// Reveal hidden spikes near the player
void Level_RevealTraps(float player_x, float player_y){
    int16_t player_col = (int16_t)(player_x / TILE_SIZE);
    int16_t player_row = (int16_t)(player_y / TILE_SIZE);

    for(int16_t r = player_row - 2; r <= player_row + 2; r++){ // 2 so that traps only appears when player is near
        for(int16_t c = player_col - 2; c <= player_col + 2; c++){
            if(Level_GetTile(c, r) == TILE_HIDDEN_SPIKE){
                Level_SetTile(c, r, TILE_SPIKE);
            }
        }
    }
}

// Draw all tiles to the LCD buffer
void Level_Draw(int16_t camera_x){
    int16_t start_col = camera_x / TILE_SIZE;
    int16_t end_col = (camera_x + SCREEN_WIDTH) / TILE_SIZE + 1;
    if(start_col < 0){
        start_col = 0;
    }
    if(end_col > LEVEL_WIDTH){
        end_col = LEVEL_WIDTH;
    }

    for(int16_t row = 0; row < LEVEL_HEIGHT; row++){
        for(int16_t col = start_col; col < end_col; col++){
            uint8_t tile = level_map[row][col];
            if(tile == TILE_AIR || tile == TILE_HIDDEN_SPIKE){
                continue;
            }

            int16_t sx = col * TILE_SIZE - camera_x;
            int16_t sy = row * TILE_SIZE;

            switch(tile){
                case TILE_GROUND:
                    LCD_Draw_Rect(sx, sy, TILE_SIZE, TILE_SIZE, COLOUR_GROUND, 1);
                    LCD_Draw_Line(sx, sy, sx + 15, sy, 0);
                    LCD_Draw_Line(sx, sy, sx, sy + 15, 0);
                    break;
                case TILE_BRICK:
                    LCD_Draw_Rect(sx, sy, TILE_SIZE, TILE_SIZE, COLOUR_BRICK, 1);
                    LCD_Draw_Line(sx, sy + 8, sx + 15, sy + 8, 0);
                    LCD_Draw_Line(sx + 8, sy, sx + 8, sy + 7, 0);
                    LCD_Draw_Line(sx + 4, sy + 9, sx + 4, sy + 15, 0);
                    break;
                case TILE_SPIKE:
                    LCD_Draw_Rect(sx, sy + 8, TILE_SIZE, 8, COLOUR_SPIKE, 1);
                    LCD_Draw_Rect(sx + 2, sy + 4, 4, 4, COLOUR_SPIKE, 1);
                    LCD_Draw_Rect(sx + 10, sy + 4, 4, 4, COLOUR_SPIKE, 1);
                    LCD_Draw_Rect(sx + 4, sy, 2, 4, COLOUR_SPIKE, 1);
                    LCD_Draw_Rect(sx + 12, sy, 2, 4, COLOUR_SPIKE, 1);
                    break;
                case TILE_FAKE_GROUND:
                    LCD_Draw_Rect(sx, sy, TILE_SIZE, TILE_SIZE, COLOUR_FAKE_GROUND, 1);
                    LCD_Draw_Line(sx, sy, sx + 15, sy, 0);
                    LCD_Draw_Line(sx, sy, sx, sy + 15, 0);
                    break;
                case TILE_GOAL:
                    LCD_Draw_Rect(sx + 7, sy, 2, TILE_SIZE, 1, 1);
                    LCD_Draw_Rect(sx + 9, sy + 1, 6, 5, COLOUR_GOAL, 1);
                    break;
            }
        }
    }
}