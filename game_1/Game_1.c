#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg;  // Joystick configuration
extern Joystick_t joystick_data;     // Current joystick readings
/**
 * @brief Game 1 Implementation - Student can modify
 * 
 * EXAMPLE: Shows how to use PWM LED for visual feedback
 * This is a placeholder with a bouncing animation that changes LED brightness.
 * Replace this with your actual game logic!
 */

 const uint8_t Cursor[10][10] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {255, 0, 0, 0, 0, 0, 0, 0, 0, 255},
    {255, 255, 0, 0, 0, 0, 0, 0, 255, 255},
    {255, 255, 255, 0, 0, 0, 0, 255, 255, 255},
    {255, 255, 255, 255, 0, 0, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
};

int player_coord = 1;  // starts top-left
int coord_state[9] = {0}; // Clean unpressed grid
static uint8_t btn2_last = 0; // Initial state for button press (used for debouncing later)

 // State definition for enum type, will be used in FSM cases
typedef enum {
    STATE_GRID,
    STATE_RADIO
} FSM_State_t;

// Game state - customize for your game

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

MenuState Game1_Run(void) {
    // Initialize game state
    
    // Play a brief startup sound
    buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer
    
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu
    
    // Game's own loop - runs until exit condition
    while (1) {
        uint32_t frame_start = HAL_GetTick();
        
        // Read input and then joystick
        Input_Read();
        Joystick_Read(&joystick_cfg, &joystick_data);
        // Read and figure out the movement
        movement(&joystick_data);

        /*
        MOVE THE BUTTON PRESS CHECK INTO GRID FSM!!!!
        */
        // Check if button pressed
        if (current_input.btn2_pressed && !btn2_last) {
            int pos_id = player_coord - 1;
            coord_state[pos_id] = !coord_state[pos_id];
        }

        btn2_last = current_input.btn2_pressed;
        
        // UPDATE: Game logic
        
        // RENDER: Draw to LCD
        LCD_Fill_Buffer(0);
        
        // INSERT CODE FOR DRAWING HERE
        draw_grid();

        LCD_Refresh(&cfg0);
        
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME1_FRAME_TIME_MS) {
            HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
        }
    }
    
    return exit_state;  // Tell main where to go next
}

// Draw the main 3x3 grid
void draw_grid() {
    // White Background
    LCD_Draw_Rect(10, 110, 120, 120, 1, 1);

    int cell_size = 40;
    int padding = 2;  // padding between spaces

    // Draw grids of square
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {

            int x = 10 + col * cell_size + padding;
            int y = 110 + row * cell_size + padding;

            LCD_Draw_Rect(x, y,
                          cell_size - 2 * padding,
                          cell_size - 2 * padding,
                          0, 1);
        }
    }
    draw_selected_coords();
    draw_grid_cursor(player_coord);
}

void draw_grid_cursor(int player_coord) {

    int row = (player_coord - 1) / 3;
    int col = (player_coord - 1) % 3;

    int center_x = 10 + col * 40 + 20;
    int center_y = 110 + row * 40 + 20;

    int draw_x = center_x - 10;
    int draw_y = center_y - 10;

    LCD_Draw_Sprite_Colour_Scaled(draw_x, draw_y, 10, 10, (uint8_t*)Cursor, 2, 2);
}

void draw_selected_coords() {

    for (int i = 0; i < 9; i++) {

        if (coord_state[i] == 1) {

            int row = i / 3;
            int col = i % 3;

            int center_x = 10 + col * 40 + 20;
            int center_y = 110 + row * 40 + 20;

            // Draw white filled circle
            LCD_Draw_Circle(center_x, center_y, 8, 1, 1);
        }
    }
}

// Modified the global variable player_coord
void movement(Joystick_t* joy) {

    Direction dir = joy->direction;

    if (dir == N) {
        if (player_coord > 3) player_coord -= 3;
    }
    else if (dir == S) {
        if (player_coord < 7) player_coord += 3;
    }
    else if (dir == E) {
        if (player_coord % 3 != 0) player_coord += 1;
    }
    else if (dir == W) {
        if ((player_coord - 1) % 3 != 0) player_coord -= 1;
    }
}