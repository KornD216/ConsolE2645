#ifndef GAME_1_H
#define GAME_1_H
#include "Joystick.h"

#include "Menu.h"

// State Handler
void handle_state_grid(Joystick_t* joy);
void handle_state_radio(Joystick_t* joy);

// ISR Callback Handler
void Game1_HandleButton3();

// Headers for radio-drawing related elements
void draw_radio(void);
void tune_freq(Joystick_t* joy);
void draw_life(int player_health);

// Headers for grid-drawing related elements
void draw_grid(void);
void draw_grid_cursor(int player_coord);
void movement(Joystick_t* joy);
void draw_selected_coords();

/**
 * @brief Game 1 - Student can implement their own game here
 * 
 * Placeholder for Student 1's game implementation.
 * This structure allows multiple students to work on separate games
 * while sharing common utilities from the shared/ folder.
 * 
 * The menu system calls this function when Game 1 is selected.
 * The function runs its own loop and returns when the game exits.
 * 
 * @return MenuState - Where to go next (typically MENU_STATE_HOME for menu)
 */

MenuState Game1_Run(void);

#endif // GAME_1_H
