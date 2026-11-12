#include "api.h"
#include "firmware.h"

#include "logging/log.h"
#include "profiling/profiling.h"
#include "stdio_text_protocol/stdio_text_protocol.h"

#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"


typedef enum stopwatch
{
	STOPWATCH_LOOP,
	STOPWATCH_COMMAND,
	STOPWATCH_LED,
	STOPWATCH_BUTTON,
	STOPWATCH_PI,
} stopwatch;

int main()
{
	stdio_text_protocol_init();
	led_task_init();
	button_task_init();
	pi_task_init();

	profiling_init();

	profiling_stopwatch_init(STOPWATCH_LOOP, "loop");
	profiling_stopwatch_init(STOPWATCH_COMMAND, "command");
	profiling_stopwatch_init(STOPWATCH_LED, "led");
	profiling_stopwatch_init(STOPWATCH_BUTTON, "button");
	profiling_stopwatch_init(STOPWATCH_PI, "pi");

	LOG_INF("%s %s\n", FIRMWARE_NAME, FIRMWARE_VERSION);

	while (1)
	{
		profiling_start(STOPWATCH_LOOP);

		profiling_start(STOPWATCH_COMMAND);
		const command_t *command = stdio_text_protocol_handle();
		if (command != NULL)
		{
			api_handle(command);
		}
		profiling_stop(STOPWATCH_COMMAND);

		profiling_start(STOPWATCH_LED);
		led_task_handle();
		profiling_stop(STOPWATCH_LED);

		profiling_start(STOPWATCH_BUTTON);
		button_task_handle();
		profiling_stop(STOPWATCH_BUTTON);

		profiling_start(STOPWATCH_PI);
		pi_task_handle();
		profiling_stop(STOPWATCH_PI);

		profiling_stop(STOPWATCH_LOOP);
	}
}