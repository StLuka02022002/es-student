#include "button-task.h"

#include <stdint.h>

#include "led-task/led-task.h"
#include "platform/platform.h"
#include "systime/systime.h"

typedef enum
{
    RELEASE,
    IS_PRESSING,
    PRESSES,
    IS_RELEASING
} button_state_t;

static uint64_t last_press_us;
static uint32_t button_count;
static button_state_t button_state;

void button_task_init(void)
{
    last_press_us = systime_us();
    button_count = 0;
    button_state = RELEASE;
}

void button_task_handle(void)
{
    bool raw = platform_button_read();
    uint64_t now = systime_us();

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