#include "telemetry-task.h"
#include <stddef.h>

#include "platform/platform.h"
#include "systime/systime.h"


telemetry_task_callback_t telemetry_task_callback;
telemetry_state_t telemetry_state;

uint64_t period_us;
uint64_t last_toggle_us;

void telemetry_task_init(telemetry_task_callback_t on_record)
{
    if (on_record != NULL)
    {
        telemetry_task_callback = on_record;
    }
    telemetry_state = TELEMETRY_STATE_IDLE;
    last_toggle_us = systime_us();
    period_us = TELEMETRY_TASK_DEFAULT_PERIOD_MS * 1000;
}

void telemetry_task_handle(void)
{
    if (telemetry_state == TELEMETRY_STATE_IDLE)
    {
        return;
    }
    if (systime_period_elapsed(&last_toggle_us, period_us))
    {
        telemetry_record_t rec = {
            .time_us = systime_us(),
            .voltage_v = platform_voltage_read(),
            .temp_c = platform_temp_read(),
        };
        if (telemetry_task_callback)
        {
            telemetry_task_callback(&rec);
        }
    }
}

bool telemetry_task_start(uint32_t period_ms)
{
    if (period_ms < 10)
    {
        return false;
    }
    period_us = (uint64_t)period_ms * 1000;
    last_toggle_us = systime_us();
    telemetry_state = TELEMETRY_STATE_RUN;
    return true;
}

void telemetry_task_stop(void)
{
    telemetry_state = TELEMETRY_STATE_IDLE;
}

telemetry_state_t telemetry_task_get_state(void)
{
    return telemetry_state;
}

uint32_t telemetry_task_get_period_ms(void)
{
    return (uint32_t)(period_us / 1000);
}