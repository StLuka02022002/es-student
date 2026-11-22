#include "platform.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"

#define ADC_PIN 26
#define ADC_CHANNEL 0
#define ADC_TEMP_CHANNEL 4

#define ADC_VREF_V 3.3f
#define ADC_LEVELS 4096

void platform_init(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    platform_led_set(false);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_set_temp_sensor_enabled(true);
}

void platform_led_set(bool on)
{
    gpio_put(LED_PIN, on);
}

bool platform_led_read(void)
{
    return gpio_get(LED_PIN);
}

bool platform_button_read(void)
{
    return !gpio_get(BUTTON_PIN);
}

float platform_voltage_read(void){
    adc_select_input(ADC_CHANNEL);
    return adc_read() * ADC_VREF_V / ADC_LEVELS;
}

float platform_temp_read(void){
    adc_select_input(ADC_TEMP_CHANNEL);
    float voltage = adc_read() * ADC_VREF_V / ADC_LEVELS;
    return 27 - (voltage - 0.706f) / 0.001721f;
}