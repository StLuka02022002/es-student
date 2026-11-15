#include "platform.h"

#include "hardware/gpio.h"

static uint32_t platform_buttons[] = {BUTTON_PIN, BUTTON_TWO_PIN};

void platform_init(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    platform_led_set(false);
    for (size_t i = 0; i < PLATFORM_BUTTON_COUNT; i++)
    {
        gpio_init(platform_buttons[i]);
        gpio_set_dir(platform_buttons[i], GPIO_IN);
        gpio_pull_up(platform_buttons[i]);
    }
}

void platform_led_set(bool on)
{
    gpio_put(LED_PIN, on);
}

bool platform_led_read(void)
{
    return gpio_get(LED_PIN);
}

bool platform_button_read(uint32_t id, bool *result)
{
    if (result == NULL || id >= PLATFORM_BUTTON_COUNT)
        return false;
    *result = !gpio_get(platform_buttons[id]);
    return true;
}