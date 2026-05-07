#ifndef CHEF_PLAYER_H
#define CHEF_PLAYER_H

#include <stdint.h>
#include "LCD.h"
#include "KitchenEngine.h"
#include "Joystick.h" 

#define FACING_N 0
#define FACING_S 1
#define FACING_E 2
#define FACING_W 3

typedef struct ChefPlayer_t {
    uint8_t grid_x;
    uint8_t grid_y;
    int8_t held_item;
    uint8_t facing;
    uint8_t item_state;
    PlateItem_t *held_plate;  
} ChefPlayer_t;

void ChefPlayer_Init(ChefPlayer_t *player);
void ChefPlayer_Draw(ChefPlayer_t *player);
void ChefPlayer_Update(ChefPlayer_t *player, UserInput input);
void ChefPlayer_Interact(ChefPlayer_t *player);

#endif