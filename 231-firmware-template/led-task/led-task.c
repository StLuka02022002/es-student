#include "led-task.h"
#include "led.h"
#include "pico/time.h"

static uint64_t half_period_us;
static led_state_t led_state;
static uint64_t last_toggle_us;

static bool period_elapsed(uint64_t *last_us, uint64_t period_us);

void led_task_init(void)
{
    led_init();
    half_period_us = 1000 * 1000 / 2;
    last_toggle_us = time_us_64();
    led_state = LED_STATE_BLINK;
}

void led_task_handle(void)
{
    switch (led_state)
    {
    case LED_STATE_BLINK:
    {
        if (period_elapsed(&last_toggle_us, half_period_us))
        {
            led_toggle();
        }
        break;
    }
    default:
        break;
    }
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    switch (state)
    {
    case LED_STATE_OFF:
        led_set(false);
        break;
    case LED_STATE_ON:
        led_set(true);
        break;
    case LED_STATE_BLINK:
        led_set(!led_is_on());
        last_toggle_us = time_us_64();
        break;
    default:
        break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms > 0)
    {
        half_period_us = ((uint64_t)period_ms * 1000) / 2;
        last_toggle_us += half_period_us;
        return true;
    }
    return false;
}

uint32_t led_task_get_period_ms(void)
{
    return 2u * (uint32_t)(half_period_us / 1000);
}

static bool period_elapsed(uint64_t *last_us, uint64_t period_us)
{

    uint64_t now_us = time_us_64();
    if (now_us - *last_us < period_us)
    {
        return false;
    }

    *last_us += period_us;

    if (now_us - *last_us >= period_us)
    {
        *last_us = now_us;
    }

    return true;
}