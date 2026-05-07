#ifndef BUZZER_SOUNDS_H
#define BUZZER_SOUNDS_H

#include "Buzzer.h"


void Buzzer_Play(uint32_t freq, uint8_t volume, uint32_t duration_ms);
void Buzzer_Update(void);

void Sound_OrderComplete(void);
void Sound_OrderExpired(void);
void Sound_Burnt(void);
void Sound_ChopDone(void);
void Sound_CookDone(void);
void Sound_PickUp(void);
void Sound_Bin(void);

#endif