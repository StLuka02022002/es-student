#pragma once

#include <stdint.h>
#include <stddef.h>

#define BUTTON_PIN 15
#define BUTTON_TWO_PIN 14
#define PLATFORM_BUTTON_COUNT 2
#define LED_PIN 25

void platform_init(void);

void platform_led_set(bool on);
bool platform_led_read(void);

bool platform_button_read(uint32_t id, bool *result);