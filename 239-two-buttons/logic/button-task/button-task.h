#pragma once

#include <stdint.h>
#include <stddef.h>

#define DEBOUNCE_MS 20
#define DEBOUNCE_US 20 * 1000

typedef void (*button_task_callback_t)(void);

void button_task_init(uint32_t id, button_task_callback_t on_press, button_task_callback_t long_press);
void button_task_handle(void);

bool button_task_is_pressed(uint32_t id, bool *result);
bool button_task_get_press_count(uint32_t id, uint32_t *result);

size_t button_task_get_buttons_count(void);