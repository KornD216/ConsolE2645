#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include "Joystick.h"

// PHYSICS CONSTANTS
#define PLAYER_GRAVITY      0.4f    // Gravity per frame (pixel/frame^2)
#define PLAYER_JUMP_VEL    -7.0f    // Jump velocity (negative = upward)
#define PLAYER_SPEED        2.5f    // Horizontal speed (pixels/frame)
#define PLAYER_MAX_FALL     6.0f   
#define PLAYER_JOY_DEADZONE 0.2f    // Deadzone threshold, character will not move unless exceeds the deadzone

// PLAYER DIMENSIONS
#define PLAYER_WIDTH  12    // pixel
#define PLAYER_HEIGHT 18    // pixel

// PLAYER FSM STATES
typedef enum {
    PLAYER_IDLE,
    PLAYER_RUNNING,
    PLAYER_JUMPING,
    PLAYER_FALLING,
    PLAYER_DEAD
} PlayerState_t;

// PLAYER STRUCTURE
typedef struct {
    float x;                // X position
    float y;                // Y position
    float vy;               // Velocity in Y space
    PlayerState_t state;    // Current FSM state
    uint8_t on_ground;      // 1 if standing on solid tile
    uint8_t dead;
} Player_t;

// Initialise player position
void Player_Init(Player_t* player, float start_x, float start_y);
// Update player physics, input, and collision
void Player_Update(Player_t* player, Joystick_t* joy, uint8_t jump_pressed);
// Draw player at current position
void Player_Draw(Player_t* player, int16_t camera_x);


#endif