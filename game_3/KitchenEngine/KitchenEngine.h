#ifndef KITCHEN_ENGINE_H
#define KITCHEN_ENGINE_H

#include <stdint.h>
#include "LCD.h"
#include "rng.h"

typedef struct ChefPlayer_t ChefPlayer_t;

#define MAP_ROWS    8
#define MAP_COLS    10
#define TILE_SIZE   24
#define HUD_HEIGHT  48


#define TILE_FLOOR    0
#define TILE_WALL     1
#define TILE_SERVE    2
#define TILE_CHOP     3
#define TILE_STOVE    4
#define TILE_PLATE    5
#define TILE_CRATE_A  6 //tomato
#define TILE_CRATE_B  7 //lettuce
#define TILE_CRATE_C  8 //bun
#define TILE_CRATE_D  9 //patty
#define TILE_BIN      10

#define CHOP_TIME_MS     2000
#define COOK_TIME_MS     3000
#define BURN_TIME_MS     6000
#define ORDER_TIME_MS    30000
#define GAME_DURATION_MS 180000

#define SCORE_1_STAR  50
#define SCORE_2_STAR  100
#define SCORE_3_STAR  200
#define ORDER_PENALTY 50

#define MAX_STATIONS     8
#define MAX_PLATE_ITEMS  3
#define MAX_PLATES       3
#define PLATE_MAX_COUNT  3
#define MAX_RECIPES      4
#define MAX_RECIPE_ITEMS 3
#define MAX_FLOOR_ITEMS  5
#define MAX_ORDERS       3
#define MIN_ACTIVE_ORDERS 2

#define ITEM_NONE     0
#define ITEM_TOMATO   1
#define ITEM_LETTUCE  2
#define ITEM_BUN      3
#define ITEM_PATTY    4
#define ITEM_PLATE    5

#define STATE_RAW     0
#define STATE_CHOPPED 1
#define STATE_COOKED  2
#define STATE_BURNED  3


typedef struct {
    uint8_t  grid_x;
    uint8_t  grid_y;
    uint8_t  tile_type;
    uint8_t  has_item;
    uint8_t  item_type;
    uint8_t  item_state;
    uint32_t elapsed_ms;
    uint32_t duration_ms;
    uint32_t last_tick;
    uint8_t  done;
    uint8_t  burned;
    uint32_t burn_elapsed_ms;
    uint32_t burn_tick;
} StationSlot_t;


typedef struct {
    uint8_t active;
    uint8_t on_floor;
    uint8_t grid_x;
    uint8_t grid_y;
    uint8_t items[MAX_PLATE_ITEMS];
    uint8_t states[MAX_PLATE_ITEMS];
    uint8_t count;
} PlateItem_t;


typedef struct {
    uint8_t  items[MAX_RECIPE_ITEMS];
    uint8_t  states[MAX_RECIPE_ITEMS];
    uint8_t  count;
    uint16_t points;
} Recipe_t;


typedef struct {
    uint8_t active;
    uint8_t grid_x;
    uint8_t grid_y;
    uint8_t item_type;
    uint8_t item_state;
} FloorItem_t;


typedef struct {
    uint8_t  active;
    uint8_t  recipe_index;
    uint32_t start_tick;
    uint32_t elapsed_ms;
} Order_t;


extern StationSlot_t stations[MAX_STATIONS];
extern uint8_t station_count;
extern uint16_t kitchen_score;
extern const Recipe_t recipes[MAX_RECIPES];
extern FloorItem_t floor_items[MAX_FLOOR_ITEMS];
extern PlateItem_t plates[MAX_PLATES];
extern uint8_t plate_crate_count;
extern Order_t orders[MAX_ORDERS];
extern uint32_t game_start_tick;
extern uint8_t game_over_flag;


void     KitchenEngine_Init(uint8_t stage);
void     Kitchen_DrawMap(void);
void     Kitchen_DrawTile(uint8_t col, uint8_t row);
uint8_t  Kitchen_GetTile(uint8_t col, uint8_t row);
void     Kitchen_InitStations(void);
StationSlot_t* Kitchen_GetStationAt(uint8_t col, uint8_t row);
void     Kitchen_UpdateStations(ChefPlayer_t *player);
void     Kitchen_DrawProgressBars(void);
int8_t   Kitchen_CheckRecipe(PlateItem_t *plate);
void     Kitchen_DrawHUD(void);
void     Kitchen_DropItem(uint8_t col, uint8_t row, uint8_t item, uint8_t state);
FloorItem_t* Kitchen_GetFloorItem(uint8_t col, uint8_t row);
void     Kitchen_DrawFloorItems(void);
void     Kitchen_DrawPlates(void);
PlateItem_t* Kitchen_GetFloorPlate(uint8_t col, uint8_t row);
void     Kitchen_InitPlates(void);
void     Kitchen_InitOrders(void);
void     Kitchen_UpdateOrders(void);
void     Kitchen_DrawOrders(void);
int8_t   Kitchen_MatchOrder(PlateItem_t *plate);

#endif