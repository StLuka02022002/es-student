#include "api-commands.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#include "device/device.h"
#include "systime/systime.h"
#include "profiling/profiling.h"

#include "firmware.h"
#include "led-task/led-task.h"
#include "pi-task/pi-task.h"
#include "button-task/button-task.h"
#include "platform/platform.h"

#include "telemetry-task/telemetry-task.h"

static void cmd_info(const command_t *);
static void cmd_uptime(const command_t *);
static void cmd_led_enable(const command_t *);
static void cmd_led_disable(const command_t *);
static void cmd_led_blink(const command_t *);
static void cmd_led_period(const command_t *);
static void cmd_button(const command_t *);
static void cmd_pi_start(const command_t *);
static void cmd_pi(const command_t *);
static void cmd_profiling(const command_t *);
static void cmd_profiling_reset(const command_t *);
static void cmd_get_voltage(const command_t *);
static void cmd_get_temp(const command_t *);
static void cmd_tm_start(const command_t *);
static void cmd_tm_stop(const command_t *);
static void cmd_charge_on(const command_t *);
static void cmd_charge_off(const command_t *);
static void cmd_timer_blink_start(const command_t *);
static void cmd_timer_blink_stop(const command_t *);
static void cmd_cap_start(const command_t *);
static void cmd_cap_dump(const command_t *);

static void print_led_state(void);
static bool parse_u32(const char *text, uint32_t *value);
const api_command_t api_commands[] = {
    {"info", "device pasport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"led_enable", "set LED to ON state", cmd_led_enable},
    {"led_disable", "set LED to OFF state", cmd_led_disable},
    {"led_blink", "set LED to BLINK state", cmd_led_blink},
    {"led_period", "set blink period", cmd_led_period},
    {"button", "button state and press count", cmd_button},
    {"pi_start", "start pi calculation", cmd_pi_start},
    {"pi", "pi calculation result or state", cmd_pi},
    {"profiling", "stopwatch results", cmd_profiling},
    {"profiling_reset", "reset stopwatch results", cmd_profiling_reset},
    {"get_voltage", "voltage on GP26, ADC input 0", cmd_get_voltage},
    {"get_temp", "chip temperature, degrees Celsius", cmd_get_temp},
    {"tm_start", "start telemety", cmd_tm_start},
    {"tm_stop", "stop telemetry", cmd_tm_stop},
    {"charge_on", "charge on", cmd_charge_on},
    {"charge_off", "charge off", cmd_charge_off},
    {"timer_blink_start", "blink led by timer: timer_blink_start <period_ms>", cmd_timer_blink_start},
    {"timer_blink_stop", "stop timer blink, number of toggles", cmd_timer_blink_stop},
    {"cap_start", "start capture ADC", cmd_cap_start},
    {"cap_dump", "dump capture ADC", cmd_cap_dump},
};

void api_print_telemetry(const telemetry_record_t *record)
{
    printf("tm: %llu %.3f %.1f\n", record->time_us, record->voltage_v, record->temp_c);
}

const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);

static void cmd_info(const command_t *command)
{
    device_info_t info = {0};
    device_get_info(&info);
    printf("name: %s\n", FIRMWARE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("project: %s\n", FIRMWARE_PROJECT);
    printf("repo: %s\n", FIRMWARE_REPO);
    printf("board: %s\n", info.board);
    printf("serial: %s\n", &info.board_id[0]);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
           (unsigned)info.manufacturer, (unsigned)info.part, (unsigned)info.revision);
    printf("pico-sdk: %s\n", info.pico_sdk);
}

static void cmd_uptime(const command_t *)
{
    printf("uptime: %llu ms\n", systime_ms());
}

static void cmd_led_enable(const command_t *)
{
    led_task_set_state(LED_STATE_ON);
    print_led_state();
}

static void cmd_led_disable(const command_t *)
{
    led_task_set_state(LED_STATE_OFF);
    print_led_state();
}

static void cmd_led_blink(const command_t *)
{
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc < 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }
    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    print_led_state();
}

static void cmd_timer_blink_start(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc < 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage timer_blink_start <period_ms>\n");
        return;
    }
    if (!platform_blink_start(period_ms))
    {
        printf("error: period_ms must be 10..60000\n");
        return;
    }
    led_task_set_state(LED_STATE_OFF);

    printf("timer_blink: started, period %u ms\n", period_ms);
}

static void cmd_timer_blink_stop(const command_t *command)
{
    uint32_t toggles = platform_blink_stop();
    printf("timer_blink: stopped, %u toggles\n", toggles);
}

static void print_led_state(void)
{
    printf("led: ");
    switch (led_task_get_state())
    {
    case LED_STATE_ON:
    {
        printf("on");
        break;
    }
    case LED_STATE_OFF:
    {
        printf("off");
        break;
    }
    case LED_STATE_BLINK:
    {
        printf("blink");
        break;
    }
    }
    printf(", period %u ms\n", (unsigned)led_task_get_period_ms());
}

static void cmd_button(const command_t *)
{
    printf("button: %s, presses %u\n",
           button_task_is_pressed() ? "pressed" : "released", (unsigned)button_task_get_press_count());
}

static void cmd_pi_start(const command_t *command)
{
    uint32_t terms = 0;

    if (command->argc == 0)
    {
        terms = pi_task_get_terms();
    }
    else if (command->argc > 1 || !parse_u32(command->argv[0], &terms))
    {
        printf("error: usage pi_start [<terms>]\n");
        return;
    }

    if (!pi_task_set_terms(terms))
    {
        printf("error: terms must be greater than 0\n");
        return;
    }

    if (!pi_task_start())
    {
        printf("error: pi is busy\n");
        return;
    }

    printf("pi: started, %lu terms\n", pi_task_get_terms());
}

static void cmd_pi(const command_t *)
{
    switch (pi_task_get_state())
    {
    case NOT_START:
        printf("pi: not started\n");
        break;
    case CALCUTATE:
        printf("pi: running\n");
        break;
    case FINISH:
        printf("pi: %.9f in %lu ms\n", pi_task_get_result(), pi_task_get_time_calculate());
        break;
    default:
        break;
    }
}

static void cmd_profiling(const command_t *)
{
    if (!PROFILING_ENABLED)
    {
        printf("profiling is disabled in this build");
        return;
    }
    printf("profiling\n");
    for (size_t i = 0; i < COUNT_PROFILING_STOPWATCH; i++)
    {
        profiling_result_t result = {0};
        if (profiling_get(i, &result))
        {
            double mean = result.count
                              ? (double)result.avg_sum_us / (double)result.count
                              : 0.0;

            printf("%-10s count %lu   mean %.2f us   max %lu us\n",
                   result.name, result.count, mean, result.max_us);
        }
    }
}

static void cmd_profiling_reset(const command_t *)
{
    if (!PROFILING_ENABLED)
    {
        printf("profiling is disabled in this build");
        return;
    }
    profiling_reset();
    printf("profiling reset\n");
}

static bool parse_u32(const char *text, uint32_t *value)
{
    if (text == NULL || *text == '\0')
    {
        return false;
    }

    if (!isdigit((unsigned char)text[0]))
    {
        return false;
    }

    char *end = NULL;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (end == text || *end != '\0' || errno == ERANGE)
    {
        return false;
    }

    if (number > UINT32_MAX)
    {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}

static void cmd_get_voltage(const command_t *command)
{
    if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
    float voltage = platform_voltage_read();
    printf("voltage: %.3f V\n", voltage);
}

static void cmd_get_temp(const command_t *command)
{
    if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
    float temp = platform_temp_read();
    printf("temp: %.1f C\n", temp);
}

static void cmd_tm_start(const command_t *command)
{
    if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
    uint32_t period_ms = 0;

    if (command->argc == 0)
    {
        period_ms = TELEMETRY_TASK_DEFAULT_PERIOD_MS;
    }
    else if (command->argc > 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage tm_start [<period_ms>]\n");
        return;
    }

    if (!telemetry_task_start(period_ms))
    {
        printf("error: period_ms must be at least 10\n");
        return;
    }

    printf("telemetry: started, period %lu ms\n", telemetry_task_get_period_ms());
}

static void cmd_tm_stop(const command_t *command)
{
    telemetry_task_stop();

    printf("telemetry: stopped\n");
}

static void cmd_charge_on(const command_t *command)
{
    platform_charge_set(true);
    printf("charge: on\n");
}

static void cmd_charge_off(const command_t *command)
{
    platform_charge_set(false);
    printf("charge: off\n");
}

static void cmd_cap_start(const command_t *command)
{
    uint32_t count;
    uint32_t period_ms;

    if (command->argc != 2 || !parse_u32(command->argv[0], &count) || !parse_u32(command->argv[1], &period_ms))
    {
        printf("error: usage cap_start <N> <period_us>\n");
        return;
    }

    if (telemetry_task_get_state == TELEMETRY_STATE_RUN || platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }

    if (!platform_capture_start(count, period_ms))
    {
        printf("error: N must be 1..2000, period_us at least 10\n");
        return;
    }

    printf("capture: started, %u samples, period %u us\n", count, period_ms);
}

static void cmd_cap_dump(const command_t *)
{
    if (!platform_capture_done())
    {
        printf("error: no capture\n");
        return;
    }

    platform_capture_t capture = {0};
    platform_capture_get(&capture);
    printf("capture: %u samples, period %u us, interval %u..%u us, handler %u %%\n", capture.count,
           capture.period_us, capture.min_interval_us, capture.max_interval_us, (uint32_t)(capture.handler_percent * 100));
    int32_t count = 0;
    printf("cap:");
    while (count < capture.count)
    {
        if (count % 16 == 0 && count != 0)
        {
            printf("\n%u", capture.samples[count]);
        }
        else
        {
            printf(" %u", capture.samples[count]);
        }
        count++;
    }
    printf("\n");
}