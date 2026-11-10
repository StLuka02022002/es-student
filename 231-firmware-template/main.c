#include "pico/stdlib.h"
#include "stdio-text-protocol.h"
#include "api.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"

int main()
{
	stdio_text_protocol_init();
	api_init();
	led_task_init();
	button_task_init();
	pi_task_init();

	while (1)
	{
		const command_t *command = stdio_text_protocol_handle();
		if (command != NULL)
		{
			api_handle(command);
		}
		api_update();
		led_task_handle();
		button_task_handle();
		pi_task_handle();
	}
}