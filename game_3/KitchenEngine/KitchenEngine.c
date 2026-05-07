#include "main.h"     
#include <stdlib.h>
#include <stdio.h>
#include "Buzzer.h"
#include "ChefPlayer.h"
#include "KitchenEngine.h"
#include "Sprites.h"
#include "BuzzerSounds.h"
#include "rng.h"

static const uint8_t stage1_map[MAP_ROWS][MAP_COLS] = {
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1},  // row 0 - serving station
    {3, 0, 0, 0, 0, 0, 0, 0, 0, 3},  // row 1 - chopping boards
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 4},  // row 2 - stoves
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},  // row 3 - walls
    {1, 0, 0, 0, 5, 0, 0, 0, 0,10},  // row 4 - plate crate
    {6, 0, 0, 0, 0, 0, 0, 0, 0, 7},  // row 5 - tomato and lettuce
    {8, 0, 0, 0, 0, 0, 0, 0, 0, 9},  // row 6 - bun and patty 
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  // row 7 - bottom wall
};

static uint8_t current_map[MAP_ROWS][MAP_COLS];

StationSlot_t stations[MAX_STATIONS];
uint8_t station_count = 0;

uint16_t kitchen_score = 200;

PlateItem_t plates[MAX_PLATES];
uint8_t plate_crate_count = PLATE_MAX_COUNT;
Order_t orders[MAX_ORDERS];

const Recipe_t recipes[MAX_RECIPES] = {
    // Salad: chopped tomato + chopped lettuce
    { {ITEM_TOMATO,  ITEM_LETTUCE, 0},
      {STATE_CHOPPED, STATE_CHOPPED, 0}, 2, 100 },

    // Soup: cooked tomato + cooked lettuce
    { {ITEM_TOMATO,  ITEM_LETTUCE, 0},
      {STATE_COOKED,  STATE_COOKED,  0}, 2, 150 },

    // Burger: cooked patty + chopped lettuce + raw bun
    { {ITEM_PATTY,   ITEM_LETTUCE,  ITEM_BUN},
      {STATE_COOKED,  STATE_CHOPPED, STATE_RAW}, 3, 200 },

    // Hot dog: cooked patty + raw bun
    { {ITEM_PATTY,  ITEM_BUN,  0},
      {STATE_COOKED, STATE_RAW, 0}, 2, 120 },
};

FloorItem_t floor_items[MAX_FLOOR_ITEMS] = {0};

void Kitchen_InitPlates(void) {
    for (int i = 0; i < MAX_PLATES; i++) {
        plates[i].active   = 0;
        plates[i].on_floor = 0;
        plates[i].count    = 0;
    }
    plate_crate_count = PLATE_MAX_COUNT;
}
void Kitchen_InitOrders(void) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        orders[i].active       = 0;
        orders[i].recipe_index = 0;
        orders[i].start_tick   = 0;
        orders[i].elapsed_ms   = 0;
    }

    uint32_t now = HAL_GetTick();
    for (int i = 0; i < MIN_ACTIVE_ORDERS; i++) {
        uint32_t rng_val = 0;
        HAL_RNG_GenerateRandomNumber(&hrng, &rng_val);
        orders[i].active       = 1;
        orders[i].recipe_index = rng_val % MAX_RECIPES;
        orders[i].start_tick   = now;
        orders[i].elapsed_ms   = 0;
    }
}

void Kitchen_UpdateOrders(void) {
    uint32_t now = HAL_GetTick();
    uint8_t active_count = 0;

    for (int i = 0; i < MAX_ORDERS; i++) {
        if (!orders[i].active) continue;
        orders[i].elapsed_ms = now - orders[i].start_tick;

        // Check if order expired
        if (orders[i].elapsed_ms >= ORDER_TIME_MS) {
            orders[i].active = 0;
            if (kitchen_score >= ORDER_PENALTY)
                kitchen_score -= ORDER_PENALTY;
            else
                kitchen_score = 0;
            Sound_OrderExpired();
        } else {
            active_count++;
        }
    }

    // Ensure minimum active orders
        if (active_count < MIN_ACTIVE_ORDERS) {
            for (int i = 0; i < MAX_ORDERS; i++) {
                if (!orders[i].active) {
                    uint32_t rng_val = 0;
                    HAL_RNG_GenerateRandomNumber(&hrng, &rng_val);
                    orders[i].active       = 1;
                    orders[i].recipe_index = rng_val % MAX_RECIPES;
                    orders[i].start_tick   = now;
                    orders[i].elapsed_ms   = 0;
                    active_count++;
                    if (active_count >= MIN_ACTIVE_ORDERS) break;
                }
            }
        }
}




int8_t Kitchen_MatchOrder(PlateItem_t *plate) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (!orders[i].active) continue;
        const Recipe_t *recipe = &recipes[orders[i].recipe_index];
        if (plate->count != recipe->count) continue;

        uint8_t matched[MAX_PLATE_ITEMS] = {0};
        uint8_t all_match = 1;

        for (int ri = 0; ri < recipe->count; ri++) {
            uint8_t found = 0;
            for (int pi = 0; pi < plate->count; pi++) {
                if (!matched[pi] &&
                    plate->items[pi]  == recipe->items[ri] &&
                    plate->states[pi] == recipe->states[ri]) {
                    matched[pi] = 1;
                    found = 1;
                    break;
                }
            }
            if (!found) { all_match = 0; break; }
        }

        if (all_match) return i; // return order index
    }
    return -1;
}


PlateItem_t* Kitchen_GetFloorPlate(uint8_t col, uint8_t row) {
    for (int i = 0; i < MAX_PLATES; i++) {
        if (plates[i].active && plates[i].on_floor &&
            plates[i].grid_x == col && plates[i].grid_y == row) {
            return &plates[i];
        }
    }
    return NULL;
}

void Kitchen_DrawPlates(void) {
    for (int i = 0; i < MAX_PLATES; i++) {
        if (!plates[i].active || !plates[i].on_floor) continue;

        int px = plates[i].grid_x * TILE_SIZE;
        int py = HUD_HEIGHT + plates[i].grid_y * TILE_SIZE;
        LCD_Draw_Circle(px + TILE_SIZE/2, py + TILE_SIZE/2, TILE_SIZE/2 - 3, 13, 1);

        uint8_t count = plates[i].count;
        for (int j = 0; j < count; j++) {
            uint8_t colour;
            if (plates[i].states[j] == STATE_RAW) {
                switch (plates[i].items[j]) {
                    case ITEM_TOMATO:  colour = 2;  break;
                    case ITEM_LETTUCE: colour = 3;  break;
                    case ITEM_BUN:     colour = 5;  break;
                    case ITEM_PATTY:   colour = 15; break;
                    default:           colour = 1;  break;
                }
            } else if (plates[i].states[j] == STATE_CHOPPED) {
                colour = 1;
            } else {
                colour = 10;
            }
            int dot_x = px + 4 + (j * 7);
            int dot_y = py + TILE_SIZE/2;
            LCD_Draw_Circle(dot_x, dot_y, 2, colour, 1);
        }
    }
}


void Kitchen_InitStations(void) {
    station_count = 0;
    for (int r = 0; r < MAP_ROWS; r++) {
        for (int c = 0; c < MAP_COLS; c++) {
            uint8_t t = current_map[r][c];
            if (t == TILE_CHOP || t == TILE_STOVE) {
                stations[station_count].grid_x      = c;
                stations[station_count].grid_y      = r;
                stations[station_count].tile_type   = t;
                stations[station_count].has_item    = 0;
                stations[station_count].item_type   = 0;
                stations[station_count].item_state  = 0;
                stations[station_count].elapsed_ms  = 0;
                stations[station_count].duration_ms = (t == TILE_CHOP) ? CHOP_TIME_MS : COOK_TIME_MS;
                stations[station_count].last_tick   = 0;
                stations[station_count].done        = 0;
                stations[station_count].burned        = 0;
                stations[station_count].burn_elapsed_ms = 0;
                stations[station_count].burn_tick     = 0;
                station_count++;
                if (station_count >= MAX_STATIONS) return;
            }
        }
    }
}

StationSlot_t* Kitchen_GetStationAt(uint8_t col, uint8_t row) {
    for (int i = 0; i < station_count; i++) {
        if (stations[i].grid_x == col && stations[i].grid_y == row)
            return &stations[i];
    }
    return NULL;
}

void Kitchen_UpdateStations(ChefPlayer_t *player) {
    uint32_t now = HAL_GetTick();

    for (int i = 0; i < station_count; i++) {
        StationSlot_t *s = &stations[i];
        if (!s->has_item) continue;

        if (!s->done) {
            uint8_t adjacent = 0;
            int8_t dx = (int8_t)player->grid_x - (int8_t)s->grid_x;
            int8_t dy = (int8_t)player->grid_y - (int8_t)s->grid_y;
            if ((dx == 0 && (dy == 1 || dy == -1)) ||
                (dy == 0 && (dx == 1 || dx == -1))) {
                adjacent = 1;
            }

            if (s->tile_type == TILE_CHOP) {
                if (adjacent) {
                    if (s->last_tick != 0) s->elapsed_ms += (now - s->last_tick);
                    s->last_tick = now;
                } else {
                    s->last_tick = 0;
                }
            }

            if (s->tile_type == TILE_STOVE) {
                if (s->last_tick != 0) s->elapsed_ms += (now - s->last_tick);
                s->last_tick = now;
            }

            if (s->elapsed_ms >= s->duration_ms) {
                s->elapsed_ms = s->duration_ms;
                s->done = 1;
                if (s->tile_type == TILE_CHOP)  { s->item_state = STATE_CHOPPED; Sound_ChopDone(); }
                if (s->tile_type == TILE_STOVE) { s->item_state = STATE_COOKED;  Sound_CookDone(); }
            }
        }

        if (s->done && !s->burned && s->tile_type == TILE_STOVE) {
            uint32_t now_burn = HAL_GetTick();
            if (s->burn_tick == 0) {
                s->burn_tick = now_burn;
            } else {
                s->burn_elapsed_ms = now_burn - s->burn_tick;
            }
            if (s->burn_elapsed_ms >= BURN_TIME_MS) {
                if (!s->burned) {  // only trigger once
                    s->burned = 1;
                    s->item_state = STATE_BURNED;
                    Sound_Burnt();
                }
            }
        }
    }
}

void Kitchen_DrawProgressBars(void) {
    for (int i = 0; i < station_count; i++) {
        StationSlot_t *s = &stations[i];
        int x = s->grid_x * TILE_SIZE;
        int y = HUD_HEIGHT + s->grid_y * TILE_SIZE;
        int bar_x = x;
        int bar_y = y + TILE_SIZE - 4;

        if (s->has_item) {
            Kitchen_DrawTile(s->grid_x, s->grid_y);

            if (s->burned) {
                LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, 4, 0, 1);
            } else {
                switch (s->item_type) {
                    case ITEM_TOMATO:
                        if (s->item_state == STATE_CHOPPED)
                            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_tomato_chopped);
                        else if (s->item_state == STATE_COOKED)
                            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_tomato_cooked);
                        else
                            LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, 4, 2, 1);
                        break;
                    case ITEM_LETTUCE:
                        if (s->item_state == STATE_CHOPPED)
                            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_lettuce_chopped);
                        else if (s->item_state == STATE_COOKED)
                            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_lettuce_cooked);
                        else
                            LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, 4, 3, 1);
                        break;
                    case ITEM_PATTY:
                        if (s->item_state == STATE_COOKED)
                            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_patty_cooked);
                        else
                            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_patty);
                        break;
                    case ITEM_BUN:
                        LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, 4, 12, 1);
                        break;
                    default:
                        LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, 4, 1, 1);
                        break;
                }
            }
        }
        // progress bar
        if (s->has_item && !s->done && s->elapsed_ms > 0) {
            uint8_t progress = (uint8_t)((s->elapsed_ms * 100) / s->duration_ms);
            LCD_Draw_Rect(bar_x, bar_y, TILE_SIZE, 3, 13, 1);
            int fill = (TILE_SIZE * progress) / 100;
            if (fill > 0)
                LCD_Draw_Rect(bar_x, bar_y, fill, 3, 3, 1);
        }

        // burnbar
        if (s->done && !s->burned && s->tile_type == TILE_STOVE && s->burn_elapsed_ms > 0) {
            uint8_t burn_progress = (uint8_t)((s->burn_elapsed_ms * 100) / BURN_TIME_MS);
            LCD_Draw_Rect(bar_x, bar_y, TILE_SIZE, 3, 13, 1);
            int fill2 = (TILE_SIZE * burn_progress) / 100;
            if (fill2 > 0)
                LCD_Draw_Rect(bar_x, bar_y, fill2, 3, 2, 1);
        }
    }
}

int8_t Kitchen_CheckRecipe(PlateItem_t *plate) {
    for (int r = 0; r < MAX_RECIPES; r++) {
        const Recipe_t *recipe = &recipes[r];
        if (plate->count != recipe->count) continue;

        uint8_t matched[MAX_PLATE_ITEMS] = {0};
        uint8_t all_match = 1;

        for (int ri = 0; ri < recipe->count; ri++) {
            uint8_t found = 0;
            for (int pi = 0; pi < plate->count; pi++) {
                if (!matched[pi] &&
                    plate->items[pi]  == recipe->items[ri] &&
                    plate->states[pi] == recipe->states[ri]) {
                    matched[pi] = 1;
                    found = 1;
                    break;
                }
            }
            if (!found) { all_match = 0; break; }
        }

        if (all_match) return r;
    }
    return -1;
}

void Kitchen_DrawScore(void) {
    char buf[32];
    LCD_Draw_Rect(0, 0, 240, HUD_HEIGHT, 0, 1);
    sprintf(buf, "Score:%d Plates:%d", kitchen_score, plate_crate_count);
    LCD_printString(buf, 2, 4, 1, 1);
}

void KitchenEngine_Init(uint8_t stage) {
    for (int r = 0; r < MAP_ROWS; r++)
        for (int c = 0; c < MAP_COLS; c++)
            current_map[r][c] = stage1_map[r][c];
}

void Kitchen_DrawMap(void) {
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            int x = col * TILE_SIZE;
            int y = HUD_HEIGHT + (row * TILE_SIZE);

            switch (current_map[row][col]) {
                case TILE_FLOOR:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_floor);
                    break;
                case TILE_WALL:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_wall);
                    break;
                case TILE_SERVE:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_serve);
                    break;
                case TILE_CHOP:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_chopboard);
                    break;
                case TILE_STOVE:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_stove);
                    break;
                case TILE_PLATE:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_plate);
                    break;
                case TILE_CRATE_A:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_crate_tomato);
                    break;
                case TILE_CRATE_B:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_crate_lettuce);
                    break;
                case TILE_CRATE_C:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_crate_bun);
                    break;
                case TILE_CRATE_D:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_crate_patty);
                    break;
                case TILE_BIN:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_bin);
                    break;
                default:
                    LCD_Draw_Rect(x, y, TILE_SIZE, TILE_SIZE, 0, 1);
                    break;
            }
        }
    }
}

void Kitchen_DrawTile(uint8_t col, uint8_t row) {
    int x = col * TILE_SIZE;
    int y = HUD_HEIGHT + (row * TILE_SIZE);

    switch (current_map[row][col]) {
        case TILE_FLOOR:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_floor);
            break;
        case TILE_WALL:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_wall);
            break;
        case TILE_SERVE:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_serve);
            break;
        case TILE_CHOP:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_chopboard);
            break;
        case TILE_STOVE:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_stove);
            break;
        case TILE_PLATE:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_plate);
            break;
        case TILE_CRATE_A:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_crate_tomato);
            break;
        case TILE_CRATE_B:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_crate_lettuce);
            break;
        case TILE_CRATE_C:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_crate_bun);
            break;
        case TILE_CRATE_D:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_crate_patty);
            break;
        case TILE_BIN:
            LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_bin);
            break;
        default:
            LCD_Draw_Rect(x, y, TILE_SIZE, TILE_SIZE, 0, 1);
            break;
    }
}

uint8_t Kitchen_GetTile(uint8_t col, uint8_t row) {
    if (col >= MAP_COLS || row >= MAP_ROWS) return TILE_WALL;
    return current_map[row][col];
}

void Kitchen_DropItem(uint8_t col, uint8_t row, uint8_t item, uint8_t state) {
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        if (!floor_items[i].active) {
            floor_items[i].active     = 1;
            floor_items[i].grid_x     = col;
            floor_items[i].grid_y     = row;
            floor_items[i].item_type  = item;
            floor_items[i].item_state = state;
            return;
        }
    }
}

FloorItem_t* Kitchen_GetFloorItem(uint8_t col, uint8_t row) {
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        if (floor_items[i].active &&
            floor_items[i].grid_x == col &&
            floor_items[i].grid_y == row) {
            return &floor_items[i];
        }
    }
    return NULL;
}

void Kitchen_DrawFloorItems(void) {
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        if (!floor_items[i].active) continue;
        int x = floor_items[i].grid_x * TILE_SIZE;
        int y = HUD_HEIGHT + (floor_items[i].grid_y * TILE_SIZE);

        Kitchen_DrawTile(floor_items[i].grid_x, floor_items[i].grid_y);
        if (floor_items[i].item_state == STATE_BURNED) {
            Kitchen_DrawTile(floor_items[i].grid_x, floor_items[i].grid_y);
            LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, 8, 0, 1);
            continue;
        }
        switch (floor_items[i].item_type) {
            case ITEM_TOMATO:
                if (floor_items[i].item_state == STATE_CHOPPED)
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_tomato_chopped);
                else if (floor_items[i].item_state == STATE_COOKED)
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_tomato_cooked);
                else
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_tomato);
                break;
            case ITEM_LETTUCE:
                if (floor_items[i].item_state == STATE_CHOPPED)
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_lettuce_chopped);
                else if (floor_items[i].item_state == STATE_COOKED)
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_lettuce_cooked);
                else
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_lettuce);
                break;
            case ITEM_BUN:
                LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_bun);
                break;
            case ITEM_PATTY:
                if (floor_items[i].item_state == STATE_COOKED)
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_patty_cooked);
                else
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_patty);
                break;
            default:
                LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, 3, 1, 1);
                break;
        }
    }
}

uint32_t game_start_tick = 0;
uint8_t game_over_flag = 0;

void Kitchen_DrawHUD(void) {
    char buf[32];
    LCD_Draw_Rect(0, 0, 240, HUD_HEIGHT, 0, 1);

    // kitchen_score
    sprintf(buf, "S:%d", kitchen_score);
    LCD_printString(buf, 2, 2, 1, 1);

    // timer
    uint32_t elapsed = HAL_GetTick() - game_start_tick;
    int seconds_left = (int)((GAME_DURATION_MS - elapsed) / 1000);
    if (seconds_left < 0) seconds_left = 0;
    sprintf(buf, "T:%02d", seconds_left);
    LCD_printString(buf, 105, 2, seconds_left <= 10 ? 2 : 1, 1);

    // plate number
    sprintf(buf, "P:%d", plate_crate_count);
    LCD_printString(buf, 200, 2, 1, 1);

    // Check gameover
    if (elapsed >= GAME_DURATION_MS) {
        game_over_flag = 1;
    }

    for (int i = 0; i < MAX_ORDERS; i++) {
        int ox = i * 80;

        if (!orders[i].active) {
            LCD_printString("------", ox + 2, 28, 13, 1);
            continue;
        }

        const Recipe_t *r = &recipes[orders[i].recipe_index];

        uint8_t time_progress = (uint8_t)((orders[i].elapsed_ms * 100) / ORDER_TIME_MS);
        int bar_fill = (78 * (100 - time_progress)) / 100;
        uint8_t bar_colour = time_progress > 66 ? 2 : time_progress > 33 ? 5 : 3;
        LCD_Draw_Rect(ox, 44, 78, 3, 13, 1);
        if (bar_fill > 0)
            LCD_Draw_Rect(ox, 44, bar_fill, 3, bar_colour, 1);

            
        for (int j = 0; j < r->count; j++) {
            int sx = ox + 2 + (j * 14);
            int sy = 28;

            if (r->items[j] == ITEM_TOMATO) {
                if (r->states[j] == STATE_CHOPPED)
                    LCD_Draw_Sprite(sx, sy, 12, 12, (uint8_t*)sprite_sm_tomato_chopped);
                else if (r->states[j] == STATE_COOKED)
                    LCD_Draw_Sprite(sx, sy, 12, 12, (uint8_t*)sprite_sm_tomato_cooked);
            } else if (r->items[j] == ITEM_LETTUCE) {
                if (r->states[j] == STATE_CHOPPED)
                    LCD_Draw_Sprite(sx, sy, 12, 12, (uint8_t*)sprite_sm_lettuce_chopped);
                else if (r->states[j] == STATE_COOKED)
                    LCD_Draw_Sprite(sx, sy, 12, 12, (uint8_t*)sprite_sm_lettuce_cooked);
            } else if (r->items[j] == ITEM_PATTY) {
                LCD_Draw_Sprite(sx, sy, 12, 12, (uint8_t*)sprite_sm_patty_cooked);
            } else if (r->items[j] == ITEM_BUN) {
                LCD_Draw_Sprite(sx, sy, 12, 12, (uint8_t*)sprite_sm_bun);
            }
        }



        char buf[8];
        sprintf(buf, "%d", r->points);
        LCD_printString(buf, ox + 52, 28, 6, 1);
    }
}