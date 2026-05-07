#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "KitchenEngine.h"
#include "ChefPlayer.h"
#include "BuzzerSounds.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t  cfg0;
extern Joystick_cfg_t  joystick_cfg;
extern Joystick_t      joystick_data;

extern uint32_t game_start_tick;
extern uint8_t  game_over_flag;
extern uint16_t kitchen_score;

#define FRAME_TIME_MS 33
#define MOVE_DELAY_MS 150

MenuState Game3_Run(void) {

    LCD_Fill_Buffer(0);
    LCD_printString("Just Cooked!", 30, 80, 6, 2);
    LCD_printString("BT2: interact", 40, 120, 1, 2);
    LCD_printString("Joystick: move", 30, 145, 1, 2);
    LCD_Refresh(&cfg0);
    HAL_Delay(2000);

    KitchenEngine_Init(0);
    Kitchen_InitStations();
    Kitchen_InitPlates();
    Kitchen_InitOrders();

    ChefPlayer_t player;
    ChefPlayer_Init(&player);

    game_start_tick = HAL_GetTick();
    game_over_flag  = 0;

    LCD_Fill_Buffer(0);
    Kitchen_DrawMap();
    Kitchen_DrawFloorItems();
    Kitchen_DrawPlates();
    ChefPlayer_Draw(&player);
    Kitchen_DrawHUD();
    LCD_Refresh(&cfg0);

    uint32_t last_tick      = HAL_GetTick();
    uint32_t last_move_tick = 0;

    while (!game_over_flag) {

        uint32_t now = HAL_GetTick();

        if ((now - last_tick) < FRAME_TIME_MS) {
            HAL_Delay(1);
            continue;
        }
        last_tick = now;
        Input_Read();
        Joystick_Read(&joystick_cfg, &joystick_data);
        UserInput input = Joystick_GetInput(&joystick_data);

        Buzzer_Update();

        if (current_input.btn2_pressed) {
            ChefPlayer_Interact(&player);
            Kitchen_DrawFloorItems();
            Kitchen_DrawPlates();
            ChefPlayer_Draw(&player);
            LCD_Refresh(&cfg0);
        }

        if ((now - last_move_tick) >= MOVE_DELAY_MS) {
            uint8_t prev_x = player.grid_x;
            uint8_t prev_y = player.grid_y;

            ChefPlayer_Update(&player, input);
            last_move_tick = now;

            if (prev_x != player.grid_x || prev_y != player.grid_y) {
                Kitchen_DrawTile(prev_x, prev_y);
                Kitchen_DrawFloorItems();
                Kitchen_DrawPlates();
                ChefPlayer_Draw(&player);
                LCD_Refresh(&cfg0);
            }
        }
        Kitchen_UpdateStations(&player);
        Kitchen_UpdateOrders();

        Kitchen_DrawProgressBars();
        Kitchen_DrawHUD();
        LCD_Refresh(&cfg0);
    }

    /* Game over screen */
    LCD_Fill_Buffer(0);
    LCD_printString("Game Over!", 60, 70, 1, 2);

    char score_str[32];
    sprintf(score_str, "Score:%d", kitchen_score);
    LCD_printString(score_str, 60, 100, 1, 2);

    char star_str[16];
    if      (kitchen_score >= SCORE_3_STAR) sprintf(star_str, "* * *");
    else if (kitchen_score >= SCORE_2_STAR) sprintf(star_str, "* *");
    else if (kitchen_score >= SCORE_1_STAR) sprintf(star_str, "*");
    else                                    sprintf(star_str, ":(");
    LCD_printString(star_str, 80, 130, 6, 2);

    LCD_printString("Press BT2", 60, 170, 1, 2);
    LCD_printString("for menu", 70, 190, 1, 2);
    LCD_Refresh(&cfg0);

    while (1) {
        Input_Read();
        if (current_input.btn2_pressed) break;
        HAL_Delay(30);
    }

    return MENU_STATE_HOME;
}
