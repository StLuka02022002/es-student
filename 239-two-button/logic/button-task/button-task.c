#include "button-task.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "platform/platform.h"
#include "systime/systime.h"

#define DEBOUNCE_US 20000

typedef enum
{
    RELEASE,
    IS_PRESSING,
    PRESSES,
    IS_RELEASING
} button_state_t;

typedef struct
{
    uint64_t last_press_us;
    uint32_t button_count;
    button_state_t button_state;
    button_task_callback_t button_task_callback;
} button_task_state_t;

static button_task_state_t button_task_state[PLATFORM_BUTTON_COUNT] = {0};
static bool button_inits[PLATFORM_BUTTON_COUNT] = {false};

void button_task_init(uint32_t id, button_task_callback_t on_press)
{
    if (id < PLATFORM_BUTTON_COUNT)
    {
        button_task_state[id].last_press_us = systime_us();
        button_task_state[id].button_count = 0;
        button_task_state[id].button_state = RELEASE;
        button_task_state[id].button_task_callback = on_press;
        button_inits[id] = true;
    }
}

static void button_task_handle_one(uint32_t id, button_task_state_t *state)
{
    bool raw = false;
    if (!platform_button_read(id, &raw)){
        return;
    }

    uint64_t now = systime_us();
    switch (state->button_state)
    {
    case RELEASE:
        if (raw)
        {
            state->button_state = IS_PRESSING;
            state->last_press_us = now;
        }
        break;
    case IS_PRESSING:
        if (!raw)
            state->button_state = RELEASE;
        else if (now - state->last_press_us > DEBOUNCE_US)
        {
            state->button_state = PRESSES;
            state->button_count++;
            if (state->button_task_callback)
                state->button_task_callback();
        }
        break;
    case PRESSES:
        if (!raw)
        {
            state->button_state = IS_RELEASING;
            state->last_press_us = now;
        }
        break;
    case IS_RELEASING:
        if (raw)
            state->button_state = PRESSES;
        else if (now - state->last_press_us > DEBOUNCE_US)
            state->button_state = RELEASE;
        break;
    }
}

void button_task_handle(void)
{
    for (size_t i = 0; i < PLATFORM_BUTTON_COUNT; i++)
    {
        if (!button_inits[i])
            continue;
        button_task_handle_one(i, &button_task_state[i]);
    }
}

bool button_task_is_pressed(uint32_t id, bool *result)
{
    if (result == NULL || id >= PLATFORM_BUTTON_COUNT || !button_inits[id])
        return false;
    *result = (button_task_state[id].button_state == PRESSES ||
               button_task_state[id].button_state == IS_RELEASING);
    return true;
}

bool button_task_get_press_count(uint32_t id, uint32_t *result)
{
    if (result == NULL || id >= PLATFORM_BUTTON_COUNT || !button_inits[id])
        return false;
    *result = button_task_state[id].button_count;
    return true;
}

size_t button_task_get_buttons_count(void){
    return PLATFORM_BUTTON_COUNT;
}