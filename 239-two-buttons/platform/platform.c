#include "platform.h"

#include "hardware/gpio.h"

#define BUTTON_PIN 15
#define BUTTON_TWO_PIN 14

#define LED_PIN 25

static uint32_t platform_buttons[PLATFORM_BUTTON_COUNT];

void platform_init(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    platform_led_set(false);

    platform_buttons[PLATFORM_BUTTON_LED] = BUTTON_PIN;
    platform_buttons[PLATFORM_BUTTON_PERIOD] = BUTTON_TWO_PIN;

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