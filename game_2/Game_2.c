// Include libraries
#include "Game_2.h"
#include "Level.h"
#include "Player.h"
#include "InputHandler.h"
#include "LCD.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

// Hardware from main.c
extern ST7789V2_cfg_t cfg0;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;

// Game config
#define FRAME_MS            16      // 60 FPS
#define START_X             32.0f   // Start at tile 2 from the left
#define START_Y             178.0f  // Start at tile 2 from the bottom
#define START_LIVES         3
#define CAMERA_LEAD         80      // Cameara lead for how far from the left edge of the screen the player stays
#define FAKE_GROUND_DELAY   2       // Frames before fake ground disappears


// Game state
static Player_t player;
static int16_t camera_x;
static int8_t lives;
static uint8_t game_won;
static int8_t fake_ground_timer;
static int16_t fake_ground_col; 

// Camera
static void update_camera(void){
    int16_t target = (int16_t)player.x - CAMERA_LEAD;
    camera_x += (target - camera_x) / 4;
    if (camera_x < 0) camera_x = 0;
    int16_t max_cam = LEVEL_WIDTH * TILE_SIZE - 240;
    if (camera_x > max_cam) camera_x = max_cam;
}

// Check spikes and goal
static void check_tiles(void){
    if(player.dead){
        return;
    }

    int16_t left  = ((int16_t)player.x + 2) / TILE_SIZE;
    int16_t right = ((int16_t)player.x + PLAYER_WIDTH - 3) / TILE_SIZE;
    int16_t top   = ((int16_t)player.y + 2) / TILE_SIZE;
    int16_t bot   = ((int16_t)player.y + PLAYER_HEIGHT - 3) / TILE_SIZE;

    for(int16_t r = top; r <= bot; r++){
        for(int16_t c = left; c <= right; c++){
            uint8_t tile = Level_GetTile(c, r);
            if(tile == TILE_SPIKE){
                player.dead = 1;
                return;
            }
            if(tile == TILE_GOAL){
                game_won = 1;
                return;
            }
        }
    }
    // Check if standing on fake ground
    if(player.on_ground && fake_ground_timer < 0){
        int16_t feet_row = ((int16_t)player.y + PLAYER_HEIGHT) / TILE_SIZE;
        for(int16_t c = left; c <= right; c++){
            if(Level_GetTile(c, feet_row) == TILE_FAKE_GROUND){
                fake_ground_timer = FAKE_GROUND_DELAY; // turn on timer
                fake_ground_col = c; 
                break;
            }
        }
    }
}

// Initialise game
static void init_game(void){
    Level_Init();
    Player_Init(&player, START_X, START_Y);
    camera_x = 0;
    lives = START_LIVES;
    game_won = 0;
    fake_ground_timer = -1;

}

// Render game
static void render(void){
    LCD_Fill_Buffer(0);
    Level_Draw(camera_x);
    Player_Draw(&player, camera_x);

    // Score display
    char hud[16];
    sprintf(hud, "Lives:%d", lives);
    LCD_printString(hud, 5, 2, 1, 2);

    LCD_Refresh(&cfg0);
}

// MAIN GAME FUNCTION //
MenuState Game2_Run(void) {

    // Title
    LCD_Fill_Buffer(0);
    LCD_printString("CAT MARIO", 15, 50, 1, 4);
    LCD_printString("BT3: Jump", 30, 120, 1, 2);
    LCD_printString("BT2: Quit", 30, 150, 1, 2);
    LCD_Refresh(&cfg0);
    HAL_Delay(2000);

    // Init
    init_game();

    // Game loop
    while (1){
        uint32_t start = HAL_GetTick();

        // INPUT
        Input_Read();
        Joystick_Read(&joystick_cfg, &joystick_data);

        // BT2 = quit to menu
        if(current_input.btn2_pressed){
            return MENU_STATE_HOME;
        }

        // Reveal hidden spikes when player is nearby
        Level_RevealTraps(player.x, player.y);
 
        // Fake ground countdown - only collapse fake ground near character
        if(fake_ground_timer > 0){
            fake_ground_timer--;
        }
        if(fake_ground_timer == 0){
            int16_t feet_row = ((int16_t)player.y + PLAYER_HEIGHT) / TILE_SIZE;
            for(int16_t c = fake_ground_col - 2; c <= fake_ground_col + 2; c++){
                if(Level_GetTile(c, feet_row) == TILE_FAKE_GROUND){
                    Level_SetTile(c, feet_row, TILE_AIR);
                }
            }
            fake_ground_timer = -1; // timer inactive
        }

        Player_Update(&player, &joystick_data, current_input.btn3_pressed);
        check_tiles();
        update_camera();

        // RENDER
        render();

        // VICTORY
        if(game_won){
            LCD_Fill_Buffer(0);
            LCD_printString("YOU WIN!", 30, 80, 1, 4);
            char str[16];
            sprintf(str, "Lives:%d", lives);
            LCD_printString(str, 50, 140, 1, 3);
            LCD_Refresh(&cfg0);
            HAL_Delay(3000);

            // Wait for button
            while(1){
                Input_Read();
                if(current_input.btn3_pressed){
                    init_game();
                    break;
                }
                if(current_input.btn2_pressed){
                    return MENU_STATE_HOME;
                }
                HAL_Delay(30);
            }
        }

        // DEATH
        if(player.dead){
            lives--;

            LCD_Fill_Buffer(0);
            char str[16];
            sprintf(str, "Lives:%d", lives);
            LCD_printString(str, 50, 100, 1, 3);
            LCD_Refresh(&cfg0);
            HAL_Delay(1000);

            // Always respawn - no game over
            Level_Init();
            Player_Init(&player, START_X, START_Y);
            camera_x = 0;
            fake_ground_timer = -1;
        }

        // Frame timing
        uint32_t elapsed = HAL_GetTick() - start;
        if(elapsed < FRAME_MS){
            HAL_Delay(FRAME_MS - elapsed);
        }
    }
}