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

 // Upside Down Cursor
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
int player_frequency = 250; // (Ranges from 100-500)
int player_health = 3; // starts with 3 attempts remaining
int coord_state[9] = {0}; // Clean unpressed grid
int true_coord[9] = {0}; // Clean solution grid
static uint8_t btn2_last = 0; // Initial state for button press (used for debouncing later)

 // State definition will be used during the main game
typedef enum {
    STATE_GRID,
    STATE_RADIO,
    STATE_SUBMIT
} FSM_State_t;
int STATE_COUNT = 3;

// State definition not related to the main interface
typedef enum {
    STATE_START,
    STATE_PLAYING,
    STATE_CHECKING,
    STATE_CORRECT,
    STATE_WRONG,
    STATE_END
} EXTRA_State_t;

// Game state initialization
volatile FSM_State_t g_current_state = STATE_GRID;
volatile EXTRA_State_t extra_state = STATE_PLAYING;


// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

MenuState Game1_Run(void) {
    // Initialize game state
    
    // Play a brief startup sound
    buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer
    
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu
    
    // Game Main Loop While Playing
    while (1){
        uint32_t frame_start = HAL_GetTick();
        
        // Read input and then joystick
        Input_Read();
        Joystick_Read(&joystick_cfg, &joystick_data);

        // MAIN FSM LOOP
        switch(extra_state){
            case STATE_PLAYING:
                switch (g_current_state){
                case STATE_GRID:
                    handle_state_grid(&joystick_data);
                    break;

                case STATE_RADIO:
                    handle_state_radio(&joystick_data);
                    break;

                case STATE_SUBMIT:
                    handle_state_submit(&joystick_data);
                    break;
                
                default:
                    g_current_state = STATE_GRID;
                    break;
            }
            break;

            case STATE_CORRECT:
                //Display Correct Screen
                // Wait For Button Press To Continue
                // back to main game loop
                LCD_Fill_Buffer(1);
                LCD_Refresh(&cfg0);
                HAL_Delay(1000);
                extra_state = STATE_PLAYING;
                break;

            case STATE_WRONG:
                // Display Wrong Screen
                // Wait For Button Press To Continue
                // deduct health
                player_health -= 1;
                if (player_health < 1){
                    extra_state = STATE_END; //  Health is 0, switch to end screen
                    break; // Exit early to avoid the state changing to playing
                }
                LCD_Fill_Buffer(2);
                LCD_Refresh(&cfg0);
                HAL_Delay(1000);
                // back to main game loop
                extra_state = STATE_PLAYING;
                break;

            case STATE_END:
                LCD_Fill_Buffer(0);
                LCD_Refresh(&cfg0);
                HAL_Delay(1000);
                // Wait for button press to restart
                extra_state = STATE_PLAYING;
                break;

            default:
                extra_state = STATE_PLAYING;
                break;
        }
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME1_FRAME_TIME_MS) {
            HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
        }
    }
    return exit_state;  // Tell main where to go next
}

// Handler for STATE GRID
void handle_state_grid(Joystick_t* joy){
    // Read and figure out the movement
    movement(&joystick_data);
    // Clear The Screen
    LCD_Fill_Buffer(0);
    // Debouncing Mechanism
    if (current_input.btn2_pressed && !btn2_last){
            // Convert coordinate to grid number
            int pos_id = player_coord - 1;
            // Toggle the corresponding grid status
            coord_state[pos_id] = !coord_state[pos_id];
        }
    btn2_last = current_input.btn2_pressed;
    // Draw Main Elements
    draw_submit();
    draw_grid();
    draw_radio();
    // Add Player Cursors - only visible for this state
    draw_grid_cursor(player_coord);
    LCD_Refresh(&cfg0);
}

// Handler for STATE RADIO
void handle_state_radio(Joystick_t* joy){
    tune_freq(&joystick_data);
    // Clear The Screen
    LCD_Fill_Buffer(0);
    // Draw Main Elements
    draw_submit();
    draw_radio();
    draw_grid();
    LCD_Refresh(&cfg0);
}

// Handler for STATE SUBMIT
void handle_state_submit(Joystick_t* joy){
    // Clear The Screen
    LCD_Fill_Buffer(0);
    // Debouncing Mechanism
    if (current_input.btn2_pressed && !btn2_last){
            // Called submission and check - change state accordingly
            int match = check_coord();
            if (match){
                extra_state = STATE_CORRECT;
            } else {
                extra_state = STATE_WRONG;
            }
        }
    btn2_last = current_input.btn2_pressed;
    // Draw Main Elements
    draw_submit();
    draw_radio();
    draw_grid();
    LCD_Refresh(&cfg0);
}

int check_coord(){
    int match = 1;
    for (int i = 0; i < 9; i++) {
        if (true_coord[i] != coord_state[i]){
            match = 0;  // Different, set flag to False
            break;  // Early exit
        }
    }
    return match;
}

void draw_submit(){
    if (g_current_state != STATE_SUBMIT){ // GRAY, NOT SUBMITTING
        LCD_Draw_Rect(140,110, 90,40, 13,1);
        LCD_printString("SUBMIT", 150, 120, 1, 2);
    } else { // RED, ACTIVE
        LCD_Draw_Rect(140,110, 90,40, 2,1);
        LCD_printString(">SUBMIT", 145, 120, 1, 2);
    }
}
// Draw the main radio
void draw_radio(){
    // Draw main background and decide whether movement arrow will appear
    if (g_current_state != STATE_RADIO){ // ORANGE If radio is non-active, no arrows
        LCD_Draw_Rect(10,10, 220, 90, 5, 1);
        LCD_printString("KHz", 90, 60, 0, 3);
    } else { // RED If radio is active, with arrows
        LCD_Draw_Rect(10, 10, 220, 90, 2, 1);
        LCD_printString("< KHz >", 54, 60, 0, 3);
    }
    // DRAW FREQUENCY
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", player_frequency);
    LCD_printString(buffer, 90, 30, 0, 3);
}

// Tune the frequency according to player's action
void tune_freq(Joystick_t* joy){
    Direction dir = joy->direction;
    if ((dir == N || dir == E) && (player_frequency < 500)) {
        player_frequency += 10;
    }
    if ((dir == S || dir == W) && (player_frequency > 100)) {
        player_frequency -= 10;
    }
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

// ISR Handler for button 3, the main implementation is in InputHandler.C
void Game1_HandleButton3(){
      // STATE TRANSITION: Move to next state (wraps back to 0 after last state)
      // The (FSM_State_t) cast is needed because arithmetic operations (+, %) 
      // return an integer type, so we must explicitly cast back to enum type
      g_current_state = (FSM_State_t)((g_current_state + 1) % STATE_COUNT);
    }