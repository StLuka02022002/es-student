#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BUTTON_PIN 15
#define LED_PIN 25

void platform_init(void);

void platform_led_set(bool on);
bool platform_led_read(void);

bool platform_button_read(void);

float platform_voltage_read(void);

float platform_temp_read(void);

void platform_charge_set(bool on);

bool platform_blink_start(uint32_t period_ms);

uint32_t platform_blink_stop(void);