#define SIO_BASE 0xd0000000
#define GPIO_OUT_SET 0x014
#define GPIO_OUT_CLR 0x018

#include "pico/stdlib.h"
#include "hardware/gpio.h"


const uint LED_PIN = 25;

int main()
{
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	
	volatile uint32_t *gpio_out_set = (uint32_t *)(SIO_BASE + GPIO_OUT_SET);
	volatile uint32_t *gpio_out_clr = (uint32_t *)(SIO_BASE + GPIO_OUT_CLR);
	const uint32_t led_mask = 1u << LED_PIN;
	
	while(1)
	{
		*gpio_out_set = led_mask;
		sleep_ms(250);
		*gpio_out_clr = led_mask;
		sleep_ms(1000);
	}
}