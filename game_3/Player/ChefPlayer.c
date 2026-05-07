#include "ChefPlayer.h"
#include "KitchenEngine.h"
#include "Sprites.h"
#include "BuzzerSounds.h"

void ChefPlayer_Init(ChefPlayer_t *player) {
    player->grid_x     = 5;
    player->grid_y     = 5;
    player->held_item  = ITEM_NONE;
    player->item_state = STATE_RAW;
    player->facing     = FACING_S;
    player->held_plate = NULL;
}

void ChefPlayer_Draw(ChefPlayer_t *player) {
    int x = player->grid_x * TILE_SIZE;
    int y = HUD_HEIGHT + (player->grid_y * TILE_SIZE);

    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_player);

    if (player->held_plate != NULL) {
        LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/2 - 3, 13, 1);
        for (int j = 0; j < player->held_plate->count; j++) {
            uint8_t colour;
            if (player->held_plate->states[j] == STATE_RAW) {
                switch (player->held_plate->items[j]) {
                    case ITEM_TOMATO:  colour = 2;  break;
                    case ITEM_LETTUCE: colour = 3;  break;
                    case ITEM_BUN:     colour = 12; break;
                    case ITEM_PATTY:   colour = 8;  break;
                    default:           colour = 1;  break;
                }
            } else if (player->held_plate->states[j] == STATE_CHOPPED) {
                colour = 1;
            } else if (player->held_plate->states[j] == STATE_BURNED) {
                colour = 0;
            } else {
                colour = 10;
            }
            LCD_Draw_Circle(x + 4 + (j * 7), y + TILE_SIZE - 6, 2, colour, 1);
        }



    } else if (player->held_item != ITEM_NONE) {
        if (player->item_state == STATE_BURNED) {
            LCD_Draw_Circle(x + TILE_SIZE/2, y + TILE_SIZE/2, 8, 0, 1);
        } else {
            switch (player->held_item) {
                case ITEM_TOMATO:
                    if (player->item_state == STATE_CHOPPED)
                        LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_tomato_chopped);
                    else if (player->item_state == STATE_COOKED)
                        LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_tomato_cooked);
                    else
                        LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_tomato);
                    break;
                case ITEM_LETTUCE:
                    if (player->item_state == STATE_CHOPPED)
                        LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_lettuce_chopped);
                    else if (player->item_state == STATE_COOKED)
                        LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_lettuce_cooked);
                    else
                        LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_lettuce);
                    break;
                case ITEM_BUN:
                    LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_bun);
                    break;
                case ITEM_PATTY:
                    if (player->item_state == STATE_COOKED)
                        LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_patty_cooked);
                    else
                        LCD_Draw_Sprite(x, y, 24, 24, (uint8_t*)sprite_patty);
                    break;
                default: break;
            }
        }
    }
}



void ChefPlayer_Update(ChefPlayer_t *player, UserInput input) {
    uint8_t new_x = player->grid_x;
    uint8_t new_y = player->grid_y;

    switch (input.direction) {
        case N:  new_y--; player->facing = FACING_N; break;
        case S:  new_y++; player->facing = FACING_S; break;
        case E:  new_x++; player->facing = FACING_E; break;
        case W:  new_x--; player->facing = FACING_W; break;
        case NE: new_y--; new_x++; player->facing = FACING_N; break;
        case NW: new_y--; new_x--; player->facing = FACING_N; break;
        case SE: new_y++; new_x++; player->facing = FACING_S; break;
        case SW: new_y++; new_x--; player->facing = FACING_S; break;
        default: return;
    }

    if (Kitchen_GetTile(new_x, player->grid_y) == TILE_FLOOR)
        player->grid_x = new_x;
    if (Kitchen_GetTile(player->grid_x, new_y) == TILE_FLOOR)
        player->grid_y = new_y;
}

void ChefPlayer_Interact(ChefPlayer_t *player) {
    int8_t nx = player->grid_x;
    int8_t ny = player->grid_y;

    switch (player->facing) {
        case FACING_N: ny--; break;
        case FACING_S: ny++; break;
        case FACING_E: nx++; break;
        case FACING_W: nx--; break;
    }

    uint8_t tile = Kitchen_GetTile(nx, ny);
    StationSlot_t *station = Kitchen_GetStationAt(nx, ny);


    // picking up a plate from a crate
    if (tile == TILE_PLATE && player->held_plate == NULL && player->held_item == ITEM_NONE) {
        if (plate_crate_count > 0) {
            for (int i = 0; i < MAX_PLATES; i++) {
                if (!plates[i].active) {
                    plates[i].active   = 1;
                    plates[i].on_floor = 0;
                    plates[i].count    = 0;
                    player->held_plate = &plates[i];
                    plate_crate_count--;
                    Sound_PickUp();
                    return;
                }
            }
        }
        return;
    }

    // picking up food from a crate
    if (player->held_item == ITEM_NONE && player->held_plate == NULL) {
        if (tile == TILE_CRATE_A) { player->held_item = ITEM_TOMATO;  player->item_state = STATE_RAW; Sound_PickUp(); return; }
        if (tile == TILE_CRATE_B) { player->held_item = ITEM_LETTUCE; player->item_state = STATE_RAW; Sound_PickUp(); return; }
        if (tile == TILE_CRATE_C) { player->held_item = ITEM_BUN;     player->item_state = STATE_RAW; Sound_PickUp(); return; }
        if (tile == TILE_CRATE_D) { player->held_item = ITEM_PATTY;   player->item_state = STATE_RAW; Sound_PickUp(); return; }
    }


    // station interacts
    if (station != NULL) {
        if (station->has_item && station->done) {
            if (player->held_plate != NULL && !station->burned) {
                if (player->held_plate->count < MAX_PLATE_ITEMS) {
                    player->held_plate->items[player->held_plate->count]  = station->item_type;
                    player->held_plate->states[player->held_plate->count] = station->item_state;
                    player->held_plate->count++;
                    station->has_item   = 0;
                    station->done       = 0;
                    station->burned          = 0;       
                    station->burn_elapsed_ms = 0;        
                    station->burn_tick       = 0;
                    station->elapsed_ms = 0;
                    station->last_tick  = 0;
                    Kitchen_DrawTile(nx, ny);
                    return;
                }
            } else if (player->held_item == ITEM_NONE) {
                player->held_item  = station->item_type;
                player->item_state = station->item_state;
                station->has_item  = 0;
                station->done      = 0;
                station->burned          = 0;       
                station->burn_elapsed_ms = 0;        
                station->burn_tick       = 0;
                station->elapsed_ms = 0;
                station->last_tick  = 0;
                station->elapsed_ms = 0;
                station->last_tick  = 0;
                Kitchen_DrawTile(nx, ny);
                return;
            }
        }


        if (!station->has_item && player->held_item != ITEM_NONE && player->held_plate == NULL) {
            if (tile == TILE_CHOP && player->item_state == STATE_RAW
                && player->held_item != ITEM_BUN
                && player->held_item != ITEM_PATTY) {
                station->has_item   = 1;
                station->item_type  = player->held_item;
                station->item_state = player->item_state;
                station->elapsed_ms = 0;
                station->done       = 0;
                station->last_tick  = HAL_GetTick();
                player->held_item   = ITEM_NONE;
                return;
            }


            if (tile == TILE_STOVE) {
                if (player->item_state == STATE_CHOPPED ||
                   (player->item_state == STATE_RAW && player->held_item == ITEM_PATTY)) {
                    station->has_item   = 1;
                    station->item_type  = player->held_item;
                    station->item_state = player->item_state;
                    station->elapsed_ms = 0;
                    station->done       = 0;
                    station->burned          = 0;       
                    station->burn_elapsed_ms = 0;        
                    station->burn_tick       = 0;
                    station->last_tick  = HAL_GetTick();
                    player->held_item   = ITEM_NONE;
                    return;
                }
            }
        }
    }



    
    // this place food on a plate on the floor
    if (player->held_item != ITEM_NONE && player->held_plate == NULL) {
        PlateItem_t *fp = Kitchen_GetFloorPlate(player->grid_x, player->grid_y);
        if (fp != NULL && fp->count < MAX_PLATE_ITEMS) {
            fp->items[fp->count]  = player->held_item;
            fp->states[fp->count] = player->item_state;
            fp->count++;
            player->held_item  = ITEM_NONE;
            player->item_state = STATE_RAW;
            return;
        }
    }
    // bin
    if (tile == TILE_BIN) {
        if (player->held_plate != NULL) {
            player->held_plate->active   = 0;
            player->held_plate->count    = 0;
            player->held_plate->on_floor = 0;
            player->held_plate = NULL;
            plate_crate_count++;
            Sound_Bin(); 
            return;
        }
        if (player->held_item != ITEM_NONE) {
            player->held_item  = ITEM_NONE;
            player->item_state = STATE_RAW;
            Sound_Bin(); 
            return;
        }
    }
    // serving conditions
    if (tile == TILE_SERVE && player->held_plate != NULL) {
        int8_t matched_order = Kitchen_MatchOrder(player->held_plate);
        if (matched_order >= 0) {
            kitchen_score += recipes[orders[matched_order].recipe_index].points;
            orders[matched_order].active = 0;
            Sound_OrderComplete();
        }
        // Reset plate and return to crate
        player->held_plate->active   = 0;
        player->held_plate->count    = 0;
        player->held_plate->on_floor = 0;
        player->held_plate = NULL;
        plate_crate_count++;
        return;
    }

    // pick up a plate from the floor
    if (player->held_plate == NULL && player->held_item == ITEM_NONE) {
        PlateItem_t *fp = Kitchen_GetFloorPlate(player->grid_x, player->grid_y);
        if (fp != NULL) {
            fp->on_floor   = 0;
            player->held_plate = fp;
            Kitchen_DrawTile(player->grid_x, player->grid_y);
            return;
        }
    }

    // floor pick up
    if (player->held_item == ITEM_NONE && player->held_plate == NULL) {
        FloorItem_t *fi = Kitchen_GetFloorItem(player->grid_x, player->grid_y);
        if (fi != NULL) {
            player->held_item  = fi->item_type;
            player->item_state = fi->item_state;
            fi->active = 0;
            Kitchen_DrawTile(player->grid_x, player->grid_y);
            Kitchen_DrawFloorItems();
            return;
        }
    }

    // dropping plates
    if (player->held_plate != NULL && tile == TILE_FLOOR) {
        PlateItem_t *fp = Kitchen_GetFloorPlate(player->grid_x, player->grid_y);
        if (fp == NULL) {
            player->held_plate->on_floor = 1;
            player->held_plate->grid_x   = player->grid_x;
            player->held_plate->grid_y   = player->grid_y;
            player->held_plate = NULL;
        }
        return;
    }

    // dropping items
    if (player->held_item != ITEM_NONE && player->held_plate == NULL) {
        FloorItem_t *fi = Kitchen_GetFloorItem(player->grid_x, player->grid_y);
        if (fi == NULL) {
            Kitchen_DropItem(player->grid_x, player->grid_y, player->held_item, player->item_state);
            player->held_item  = ITEM_NONE;
            player->item_state = STATE_RAW;
        }
        return;
    }
}