ConsolE2645 - STM32 based video game console with 3 demo games.

## Controls

- **Joystick UP/DOWN**: Navigate menu
- **BT2 Button**: Available for custom game use
- **BT3 Button**: Select menu option or custom game use

## Hardware Features

- **STM32L476 Microcontroller**
- **ST7789V2 LCD Display** (240×320)
- **Joystick Input** with 8-directional output
- **PWM LED** for visual effects
- **Buzzer** for sound effects
- **Timers**: TIM6 (100Hz) and TIM7 (1Hz) available for game timing

See driver folders (Joystick/, PWM/, Buzzer/) for API documentation.
See [TIMER_USAGE_GUIDE.md](TIMER_USAGE_GUIDE.md) for timer examples.
