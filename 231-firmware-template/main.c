#include "pico/stdlib.h"
#include "stdio-text-protocol.h"
#include "api.h"

int main()
{
	stdio_text_protocol_init();
	api_init();

	while (1)
	{
		const command_t *command = stdio_text_protocol_handle();
		if (command != NULL)
		{
			api_handle(command);
		}
		api_update();
	}
}