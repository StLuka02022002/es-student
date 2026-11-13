#pragma once
#include <stdint.h>

#define DEBOUNCE_MS 20
#define DEBOUNCE_US 20*1000

void button_task_init(void);

void button_task_handle(void);

bool button_task_is_pressed(void);

uint32_t button_task_get_press_count(void);