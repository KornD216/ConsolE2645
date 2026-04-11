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
// Sprites
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

// Logo
const uint8_t logo[10][10] = {
    {0,0,0,255,0,0,0,0,0,0},
    {0,0,0,255,0,0,0,0,0,0},
    {0,0,0,255,0,0,0,0,0,0},
    {0,0,0,255,255,255,255,0,0,0},
    {0,0,0,255,255,255,255,0,0,0},
    {0,0,0,255,255,255,255,0,0,0},
    {0,0,0,255,255,255,255,0,0,0},
    {0,0,0,0,0,0,255,0,0,0},
    {0,0,0,0,0,0,255,0,0,0},
    {0,0,0,0,0,0,255,0,0,0}
};

// Morse Definition
const char *morse_digits[10] = {
    "-----",
    ".----",
    "..---",
    "...--",
    "....-",
    ".....",
    "-....",
    "--...",
    "---..",
    "----."
};

 // State definition will be used during the main game
typedef enum {
    STATE_GRID,
    STATE_RADIO,
    STATE_SUBMIT,
    STATE_FORFEIT
} FSM_State_t;
int STATE_COUNT = 4;

// State definition not related to the main interface
typedef enum {
    STATE_START,
    STATE_PLAYING,
    STATE_STORY,
    STATE_CORRECT,
    STATE_WRONG,
    STATE_END
} EXTRA_State_t;

morse_player morse_emitter;

int player_coord = 1;  // starts top-left
int player_frequency = 250; // (Ranges from 100-500)
int player_health = 3; // starts with 3 attempts remaining

int coord_state[9] = {0}; // Clean unpressed grid
int true_coord[9] = {0}; // Clean solution grid

int loudness = 100; // Initial Loudness of the transmission

static uint8_t btn2_last = 0; // Initial state for button press (used for debouncing later)

// Game state initialization
volatile FSM_State_t g_current_state = STATE_GRID;
volatile EXTRA_State_t extra_state = STATE_START;

// reinitialize
void reset(){
    player_coord = 1;
    player_frequency = 250;
    player_health = 3;
    // Reset the grid
    for (int i = 0; i < 9; i++){
        coord_state[i] = 0;
    }
    for (int i = 0; i < 9; i++){
        true_coord[i] = 0;
    }  
    btn2_last = 0;
    g_current_state = STATE_GRID;
    extra_state = STATE_START;
}

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

MenuState Game1_Run(void) {
    // Initialize game state
    
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu
    
    // EXAMPLE CODE FOR TESTING ONLY
    int coords[] = {1, 2, 3};
    morse_init(&morse_emitter, coords, 3, HAL_GetTick());
    // EXAMPLE CODE ENDS HERE

    // Game Main Loop While Playing
    while (1){
        uint32_t frame_start = HAL_GetTick();
        
        // Read input and then joystick
        Input_Read();
        Joystick_Read(&joystick_cfg, &joystick_data);

        // MAIN FSM LOOP
        switch(extra_state){
            case STATE_START:
                transmit_morse(&morse_emitter, frame_start);
                handle_start_screen();
                break;
            
            case STATE_STORY:
                handle_story_screen();
                break;
            
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

                case STATE_FORFEIT:
                    handle_state_forfeit(&joystick_data);
                    // Check for exit
                    if (current_input.btn2_pressed && !btn2_last){
                            // Reset everything and exit
                            reset();
                            return exit_state;
                        }
                    btn2_last = current_input.btn2_pressed;
                    break;

                default:
                    g_current_state = STATE_GRID;
                    break;
            }
            break;

            case STATE_CORRECT:
                // Display Correct Screen
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
                extra_state = STATE_START;
                reset();
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

// Start screen with monologue and glitch animation
void handle_start_screen() {
    LCD_Fill_Buffer(0);
    LCD_Draw_Sprite_Colour_Scaled(40, 170, 10, 10, (uint8_t*)logo, 1, 3);
    LCD_printString("STATION-L7", 80, 170, 1, 2);
    LCD_printString(">STANDBY", 80, 185, 13, 2);
    uint32_t time_now = HAL_GetTick();  // define current time
    static uint32_t time_start = 0;  // set to 0 only once at the start and retain updated value later.
    // Initialize only once when time_start = 0
    if (time_start == 0) {
        time_start = time_now;
    }
    // calculate time elapsed
    uint32_t time_elapsed = time_now - time_start;
    // Determine the timing cycle
    uint32_t cycle_time = time_elapsed % 2500;
    // Determine which render to use depending on the cycle timing
    if (cycle_time < 1500) {
        LCD_Draw_Rect(30,50, 180,40, 2,1);
        LCD_printString("> START <", 40, 60, 1, 3);
    } else if (cycle_time >= 1500 && cycle_time < 1650) {
        LCD_Draw_Rect(30,50, 180,20, 1,1);
        LCD_Draw_Rect(40,90, 180,20, 2,0);
        LCD_printString("!ACHTUNG!", 14, 60, 2, 5);
    } else if (cycle_time >= 1650 && cycle_time < 2300) {
        LCD_Fill_Buffer(0);
        LCD_Draw_Rect(30,50, 250,40, 1,1);
        LCD_printString("> ANFANG <", 54, 60, 0, 3);
        LCD_Draw_Sprite_Colour_Scaled(40, 170, 10, 10, (uint8_t*)logo, 2, 3);
        LCD_printString("ICH WERDE WARTEN", 90, 150, 1, 2);
        LCD_printString(">INACTIVE", 90, 200, 2, 3);
    } else {
        LCD_Draw_Rect(30,50, 180,20, 1,1);
        LCD_Draw_Rect(40,90, 180,20, 2,0);
        LCD_printString("STERBT!", 14, 60, 2, 5);
    }
    LCD_Refresh(&cfg0);
}

void handle_story_screen() {
    // Debouncing mechanism
    static int btn2_last = 0;
    static int page = 0;
    if (current_input.btn2_pressed && !btn2_last) {
        // Button was just pressed, advance to next page
        page++;
    }
    // Remember current button state for next call
    btn2_last = current_input.btn2_pressed;
    // Render Common Elements
    LCD_Fill_Buffer(0);
    LCD_Draw_Sprite_Colour_Scaled(110, 15, 10, 10, (uint8_t*)logo, 1, 1);
    LCD_printString("<BTN2> NEXT", 50, 210, 13, 2);
    // Render the current page
    switch(page) {
        case 0:
            LCD_Fill_Buffer(0);
            LCD_Draw_Sprite_Colour_Scaled(20, 120, 10, 10, (uint8_t*)logo, 1, 3);
            LCD_printString("FOR STATION L7", 60, 120, 1, 2);
            LCD_printString("YOUR ORDER", 60, 135, 13, 2);
            LCD_printString("<BTN2> NEXT", 50, 210, 13, 2);
            LCD_Refresh(&cfg0);
            break;
        case 1:
            LCD_printString(">The war has entered its worst phase.", 5, 40, 1, 1);
            LCD_printString("Not noise... but silence.", 10, 55, 1, 1);
            LCD_printString("Not what we hear,", 10, 80, 1, 1);
            LCD_printString("but what we dont.", 10, 95, 1, 1);
            LCD_printString("The Eusan Nation are on the hunt.", 10, 120, 1, 1);
            LCD_printString("Agents disappear overnight...", 10, 135, 1, 1);
            LCD_printString("Signals go dark.", 10, 160, 1, 1);
            LCD_printString("Just static...", 10, 175, 1, 1);
            LCD_Refresh(&cfg0);
            break;
        case 2:
            LCD_printString(">You operate radio for the Empire.", 5, 40, 1, 1);
            LCD_printString("Lucky... not front-lined.", 10, 55, 1, 1);
            LCD_printString("Your orders are simple:", 10, 80, 2, 1);
            LCD_printString("Transmit safe-routes.", 10, 95, 2, 1);
            LCD_printString("Guide the lost back home.", 10, 110, 2, 1);
            LCD_printString("Every message must be precise.", 10, 135, 1, 1);
            LCD_printString("Every frequency untraced.", 10, 150, 1, 1);
            LCD_Refresh(&cfg0);
            break;
        case 3:
            LCD_printString(">They are listening.", 5, 40, 1, 1);
            LCD_printString("They are always listening...", 10, 55, 1, 1);
            LCD_printString("3 mistakes will expose everything.", 10, 80, 2, 1);
            LCD_printString("the agents... or you.", 10, 95, 1, 1);
            LCD_printString("Stay focused.", 10, 120, 1, 1);
            LCD_printString("Stay quiet.", 10, 135, 1, 1);
            LCD_printString("Keep transmitting.", 10, 150, 1, 1);
            LCD_printString("GLORY TO THE EMPIRE.", 10, 170, 2, 1);
            LCD_Refresh(&cfg0);
            break;
        default:
            page = 0;
            extra_state =  STATE_PLAYING;
            break;
    }
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
    draw_forfeit();
    draw_radio();
    draw_grid();
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
    draw_forfeit();
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
    draw_forfeit();
    draw_radio();
    draw_grid();
    LCD_Refresh(&cfg0);
}

void handle_state_forfeit(Joystick_t* joy){
    // Clear The Screen
    LCD_Fill_Buffer(0);
    // Draw Main Elements
    draw_submit();
    draw_forfeit();
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

void draw_forfeit(){
    if (g_current_state != STATE_FORFEIT){ // GRAY, NOT SUBMITTING
        LCD_Draw_Rect(140,160, 90,40, 13,1);
        LCD_printString("FORFEIT", 145, 170, 1, 2);
    } else { // RED, ACTIVE
        LCD_Draw_Rect(140,160, 90,40, 2,1);
        LCD_printString(">>DEATH", 145, 170, 1, 2);
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
    // DRAW HEALTH BAR
    draw_life(player_health);
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

// Draw The Health Bar!
void draw_life(int player_health) {
    if (player_health >= 3){
        LCD_Draw_Rect(210,45, 10,40, 0,0);
    }
    if (player_health >= 2){
        LCD_Draw_Rect(195,55, 10,30, 0,0);
    }
    if (player_health >= 1){
        LCD_Draw_Rect(180,65, 10,20, 0,0);
    }
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

void morse_init(morse_player *morse_emitter, const int *coords, int length, uint32_t now)
{
    morse_emitter->coords = coords;
    morse_emitter->length = length;
    morse_emitter->digit_index = 0;
    morse_emitter->symbol_index = 0;
    morse_emitter->phase = 0;
    morse_emitter->next_time = now;
    morse_emitter->active = 1;
}

void transmit_morse(morse_player *morse_emitter, uint32_t now)
{
    if (!morse_emitter->active) return;
    if (now < morse_emitter->next_time) return;

    int digit = morse_emitter->coords[morse_emitter->digit_index];
    const char *code = morse_digits[digit];

    char symbol = code[morse_emitter->symbol_index];

    // End of full transmission
    if (digit == 0 || morse_emitter->digit_index >= morse_emitter->length) {
        buzzer_off(&buzzer_cfg);
        morse_emitter->active = 0;
        return;
    }

    // PHASE: play symbol
    if (morse_emitter->phase == 0) {
        buzzer_note(&buzzer_cfg, NOTE_C5, loudness);

        if (symbol == '.') {
            morse_emitter->next_time = now + DOT_TIME;
        } else {
            morse_emitter->next_time = now + DASH_TIME;
        }

        morse_emitter->phase = 1;
        return;
    }

    // PHASE: stop symbol + advance
    if (morse_emitter->phase == 1) {
        buzzer_off(&buzzer_cfg);

        morse_emitter->symbol_index++;

        morse_emitter->next_time = now + SYMBOL_GAP;
        morse_emitter->phase = 0;

        // end of digit
        if (code[morse_emitter->symbol_index] == '\0') {
            morse_emitter->digit_index++;
            morse_emitter->symbol_index = 0;
            morse_emitter->next_time = now + DIGIT_GAP;
        }
        return;
    }
}
// ISR Handler for button 3, the main implementation is in InputHandler.C
void Game1_HandleButton3(){
    // When In Game:
    switch (extra_state){
        case STATE_PLAYING:
            // STATE TRANSITION: Move to next state (wraps back to 0 after last state)
            // The (FSM_State_t) cast is needed because arithmetic operations (+, %) 
            // return an integer type, so we must explicitly cast back to enum type
            g_current_state = (FSM_State_t)((g_current_state + 1) % STATE_COUNT);
            break;

        case STATE_START:
            extra_state = STATE_STORY;
            break;

        default:
            break;
    }
}