#include "Player.h"
#include "Level.h"
#include "LCD.h"
#include <math.h>

// Initialise player
void Player_Init(Player_t* player, float x, float y){
    player->x = x;
    player->y = y;
    player->vy = 0;
    player->state = PLAYER_IDLE;
    player->on_ground = 0;
    player->dead = 0;
}

void Player_Update(Player_t* player, Joystick_t* joy, uint8_t jump){
    // Check if player is dead
    if(player->dead){
        return;
    }
 
    // x movement
    float move_x = 0;
    if(fabsf(joy->coord_mapped.x) > PLAYER_JOY_DEADZONE){
        move_x = joy->coord_mapped.x * PLAYER_SPEED;
    }
 
    // Jump
    if(jump && player->on_ground){
        player->vy = PLAYER_JUMP_VEL;
        player->on_ground = 0;
    }
 
    // Gravity
    player->vy += PLAYER_GRAVITY;
    if(player->vy > PLAYER_MAX_FALL){
        player->vy = PLAYER_MAX_FALL;
    }
 
    // Prevent player from moving out of bounds (x)
    player->x += move_x;
    if(player->x < 0){
        player->x = 0;
    }
    if(player->x > LEVEL_WIDTH * TILE_SIZE - PLAYER_WIDTH){
        player->x = LEVEL_WIDTH * TILE_SIZE - PLAYER_WIDTH;
    }
 
    // x collision
    int16_t top_row = ((int16_t)player->y + 1) / TILE_SIZE;
    int16_t bot_row = ((int16_t)player->y + PLAYER_HEIGHT - 2) / TILE_SIZE;
 
    if(move_x > 0) {
        int16_t col = ((int16_t)player->x + PLAYER_WIDTH - 1) / TILE_SIZE;
        for(int16_t r = top_row; r <= bot_row; r++){
            if(Level_IsSolid(col, r)){
                player->x = (float)(col * TILE_SIZE - PLAYER_WIDTH);
                break;
            }
        }
    } 
    else if(move_x < 0){
        int16_t col = (int16_t)player->x / TILE_SIZE;
        for(int16_t r = top_row; r <= bot_row; r++){
            if(Level_IsSolid(col, r)){
                player->x = (float)((col + 1) * TILE_SIZE);
                break;
            }
        }
    }
 
    // Handle landing on ground and bump into ceilings (y)
    player->y += player->vy;
    int16_t left_col = ((int16_t)player->x + 1) / TILE_SIZE;
    int16_t right_col = ((int16_t)player->x + PLAYER_WIDTH - 2) / TILE_SIZE;
 
    if(player->vy > 0) {
        int16_t row = ((int16_t)player->y + PLAYER_HEIGHT - 1) / TILE_SIZE;
        for(int16_t c = left_col; c <= right_col; c++){
            if(Level_IsSolid(c, row)){
                player->y = (float)(row * TILE_SIZE - PLAYER_HEIGHT);
                player->vy = 0;
                player->on_ground = 1;
                goto done_y;
            }
        }
        player->on_ground = 0;
    } 
    else if(player->vy < 0){
        int16_t row = (int16_t)player->y / TILE_SIZE;
        for(int16_t c = left_col; c <= right_col; c++){
            if(Level_IsSolid(c, row)){
                player->y = (float)((row + 1) * TILE_SIZE);
                player->vy = 0;
                goto done_y;
            }
        }
    }
    done_y:
 
    // Fell off bottom
    if(player->y > LEVEL_HEIGHT * TILE_SIZE){
        player->dead = 1;
    }
 
    // Update FSM state
    if(player->dead){
        player->state = PLAYER_DEAD;
    } 
    else if(!player->on_ground){
        player->state = (player->vy < 0) ? PLAYER_JUMPING : PLAYER_FALLING;
    } 
    else if(fabsf(move_x) > 0.1f){
        player->state = PLAYER_RUNNING;
    }
    else{
        player->state = PLAYER_IDLE;
    }
}

void Player_Draw(Player_t* player, int16_t camera_x){
    int16_t sx = (int16_t)player->x - camera_x;
    int16_t sy = (int16_t)player->y;
    if(sx < -PLAYER_WIDTH || sx > 240) return;
 
    // Draw player as a simple filled rectangle
    LCD_Draw_Rect(sx, sy, PLAYER_WIDTH, PLAYER_HEIGHT, 8, 1);
}