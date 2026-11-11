#include "button-task.h"
#include "pico/stdlib.h"
#include "led-task.h"
#include <stdint.h>

static uint64_t last_press_us;
static uint32_t button_count;
static button_state_t button_state;

void button_task_init(void)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    last_press_us = time_us_64();
    button_count = 0;
    button_state = RELEASE;
}

void button_task_handle(void)
{
    bool raw = !gpio_get(BUTTON_PIN);
    uint64_t now = time_us_64();

    switch (button_state)
    {
    case RELEASE:
        if (raw)
        {
            button_state = IS_PRESSING;
            last_press_us = now;
        }
        break;
    case IS_PRESSING:
        if (!raw)
        {
            button_state = RELEASE;
        }
        else if (now - last_press_us > DEBOUNCE_US)
        {
            button_state = PRESSES;
            button_count++;
            led_task_next_state();
        }
        break;
    case PRESSES:
        if (!raw)
        {
            button_state = IS_RELEASING;
            last_press_us = now;
        }
        break;
    case IS_RELEASING:
        if (raw)
        {
            button_state = PRESSES;
        }
        else if (now - last_press_us > DEBOUNCE_US)
        {
            button_state = RELEASE;
        }
        break;
    default:
        break;
    }
}

bool button_task_is_pressed(void)
{
    return (button_state == PRESSES || button_state == IS_RELEASING);
}

uint32_t button_task_get_press_count(void)
{
    return button_count;
}