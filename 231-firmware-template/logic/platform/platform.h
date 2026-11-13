#pragma once

#include <stdbool.h>

#define BUTTON_PIN 15
#define LED_PIN 25

void platform_init(void);

void platform_led_set(bool on);
bool platform_led_read(void);

bool platform_button_read(void);