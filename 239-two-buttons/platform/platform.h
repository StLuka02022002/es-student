#pragma once

#include <stdint.h>
#include <stddef.h>

#define PLATFORM_BUTTON_LED 0
#define PLATFORM_BUTTON_PERIOD 1
#define PLATFORM_BUTTON_COUNT 2

void platform_init(void);

void platform_led_set(bool on);
bool platform_led_read(void);

bool platform_button_read(uint32_t id, bool *result);