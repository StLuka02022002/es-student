#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"

const uint LED_PIN = 25;

int main()
{
   gpio_init(LED_PIN);
   gpio_set_dir(LED_PIN, GPIO_OUT);

   volatile uint32_t *gpio_out_set = (uint32_t *)(SIO_BASE + SIO_GPIO_OUT_SET_OFFSET);
   volatile uint32_t *gpio_out_clr = (uint32_t *)(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET);
   const uint32_t led_mask = 1u << LED_PIN;

   while (1)
   {
       __asm volatile ("str %1, [%0]" : : "r"(gpio_out_set), "r" (led_mask) : "memory");
       sleep_ms(250);
       __asm volatile ("str %1, [%0]" : : "r"(gpio_out_clr), "r" (led_mask) : "memory");
       sleep_ms(1000);
   }
}