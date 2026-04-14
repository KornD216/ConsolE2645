#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>

/**
 * @file Buzzer.c
 * @brief Implementation of flexible PWM buzzer driver
 * 
 * This implementation supports any timer and channel combination,
 * following the same configuration struct pattern as the Joystick
 * and LCD libraries for consistency.
 */

static inline uint32_t clamp_u32(uint32_t x, uint32_t lo, uint32_t hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void buzzer_init(Buzzer_cfg_t* cfg)
{
    if (!cfg->setup_done) {
        // Timer should be initialized by CubeMX (MX_TIMx_Init)
        // This just marks the buzzer as ready to use
        cfg->pwm_started = 0;
        cfg->setup_done = 1;
    }
}

uint8_t buzzer_is_running(Buzzer_cfg_t* cfg)
{
    return cfg->pwm_started ? 1u : 0u;
}

void buzzer_off(Buzzer_cfg_t* cfg)
{
    if (cfg->pwm_started) {
        // Make sure duty is zero first, then stop PWM fully
        __HAL_TIM_SET_COMPARE(cfg->htim, cfg->channel, 0);
        HAL_TIM_PWM_Stop(cfg->htim, cfg->channel);
        cfg->pwm_started = 0;
    }
}

void buzzer_tone(Buzzer_cfg_t* cfg, uint32_t freq_hz, uint8_t volume_percent)
{
    if (!cfg->setup_done) {
        buzzer_init(cfg);
    }

    if (volume_percent == 0 || freq_hz == 0) {
        buzzer_off(cfg);
        return;
    }

    freq_hz = clamp_u32(freq_hz, cfg->min_freq_hz, cfg->max_freq_hz);
    volume_percent = (uint8_t)clamp_u32(volume_percent, 0u, 100u);

    // NEW OPTIMIZATION HERE
    static uint32_t last_freq = 0;
    static uint8_t last_volume = 255;

    if (cfg->pwm_started &&
        last_freq == freq_hz &&
        last_volume == volume_percent)
    {
        // If nothing change, just exit
        return;
    }

    last_freq = freq_hz;
    last_volume = volume_percent;

    if (!cfg->pwm_started) {
        HAL_TIM_PWM_Start(cfg->htim, cfg->channel);
        cfg->pwm_started = 1;
    }

    uint32_t arr = (cfg->tick_freq_hz / freq_hz) - 1u;
    arr = clamp_u32(arr, 1u, 0xFFFFFFFFu);

    __HAL_TIM_SET_AUTORELOAD(cfg->htim, arr);
    __HAL_TIM_SET_COUNTER(cfg->htim, 0);

    HAL_TIM_GenerateEvent(cfg->htim, TIM_EVENTSOURCE_UPDATE);

    uint32_t half_period = (arr + 1u) / 2u;
    uint32_t ccr = (half_period * volume_percent) / 100u;

    __HAL_TIM_SET_COMPARE(cfg->htim, cfg->channel, ccr);
}

void buzzer_note(Buzzer_cfg_t* cfg, Buzzer_Note_t note, uint8_t volume_percent)
{
    // Simply play the note frequency (note enum contains the Hz value)
    buzzer_tone(cfg, (uint32_t)note, volume_percent);
}