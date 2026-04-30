#ifndef GAME_1_H
#define GAME_1_H
#include "Joystick.h"

#include "Menu.h"

void reset(void);
void draw_main_elements(void);

// State Handler
void handle_start_screen(void);
void handle_story_screen(void);
void handle_state_grid(Joystick_t* joy);
void handle_state_radio(Joystick_t* joy);
void handle_state_submit(Joystick_t* joy);
void handle_state_forfeit(Joystick_t* joy);

// ISR Callback Handler
void Game1_HandleButton3();

// Morse Related Elements
// For Morse Transmission Mechanism
#define DOT_TIME      50
#define DASH_TIME     (6 * DOT_TIME)
#define SYMBOL_GAP    300
#define DIGIT_GAP     1000

// per-frame persistent state awareness
typedef struct {
    const int *coords;
    int length;
    int digit_index; // which number in the array is being played
    int symbol_index; // which morse symbol in digit string
    uint8_t phase; // 0 = start, 1 = playing symbol, 2 = gap
    uint32_t phase_duration;
    uint32_t next_time;
    uint8_t active;
} morse_player;

void morse_init(morse_player *morse_emitter, const int *coords, int length, uint32_t now);
void transmit_morse(morse_player *morse_emitter, uint32_t now);

// Headers for submission related elements
void draw_submit(void);
int check_coord(void);

// Headers for radio related elements
void randomize_frequency(void);
void draw_radio(void);
void tune_freq(Joystick_t* joy);
void draw_life(int player_health);
void change_loudness();

// Headers for grid related elements
void draw_grid(void);
void draw_grid_cursor(int player_coord);
void movement(Joystick_t* joy);
void draw_selected_coords();

// Headers for Forfeit
void draw_forfeit(void);

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
