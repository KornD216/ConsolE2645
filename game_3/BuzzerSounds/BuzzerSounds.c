#include "BuzzerSounds.h"
#include "stm32l4xx_hal.h"

extern Buzzer_cfg_t buzzer_cfg;   
static uint32_t buzzer_stop_tick = 0;

void Buzzer_Play(uint32_t freq, uint8_t volume, uint32_t duration_ms) {
    buzzer_tone(&buzzer_cfg, freq, volume);
    buzzer_stop_tick = HAL_GetTick() + duration_ms;
}


void Buzzer_Update(void) {
    if (buzzer_stop_tick > 0 && HAL_GetTick() >= buzzer_stop_tick) {
        buzzer_off(&buzzer_cfg);
        buzzer_stop_tick = 0;
    }
}

void Sound_OrderComplete(void) { Buzzer_Play(NOTE_C5,  50, 100);  }
void Sound_OrderExpired(void)  { Buzzer_Play(NOTE_A4,  50, 500); }
void Sound_Burnt(void)        { Buzzer_Play(NOTE_B6,  50, 500); }
void Sound_ChopDone(void)      { Buzzer_Play(NOTE_E5,  50, 100); }
void Sound_CookDone(void)      { Buzzer_Play(NOTE_G5,  50, 100); }
void Sound_PickUp(void)        { Buzzer_Play(NOTE_C6,  50, 100);  }
void Sound_Bin(void)           { Buzzer_Play(NOTE_DS4, 50, 100); }