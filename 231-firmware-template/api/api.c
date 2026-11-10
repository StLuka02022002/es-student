#include "api.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "profiling.h"
#include "device.h"

#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"

static void cmd_help(const command_t *);
static void print_led_state(void);
static void blink(void);
static void calc_pi(void);
static bool parse_u32(const char *text, uint32_t *value);

void cmd_info(const command_t *);
void cmd_uptime(const command_t *);
void cmd_led_enable(const command_t *);
void cmd_led_disable(const command_t *);
void cmd_led_blink(const command_t *);
void cmd_led_period(const command_t *);
void cmd_button(const command_t *);
void cmd_pi_start(const command_t *);
void cmd_pi(const command_t *);
void cmd_profiling(const command_t *);
void cmd_profiling_reset(const command_t *);

const api_command_t commands[] = {
    {"help", "перечень команд со справкой", cmd_help},
    {"info", "паспорт прибора", cmd_info},
    {"uptime", "время с момента сброса", cmd_uptime},
    {"led_enable", "перевести светодиод в состояние «горит»", cmd_led_enable},
    {"led_disable", "перевести светодиод в состояние «выключен»", cmd_led_disable},
    {"led_blink", "перевести светодиод в состояние «мигает»", cmd_led_blink},
    {"led_period", "задать период мигания", cmd_led_period},
    {"button", "состояние кнопки и число нажатий", cmd_button},
    {"pi_start", "запустить расчёт π", cmd_pi_start},
    {"pi", "результат или состояние расчёта π", cmd_pi},
    {"profiling", "результаты секундомеров", cmd_profiling},
    {"profiling_reset", "сбросить результаты секундомеров", cmd_profiling_reset},
};

const size_t api_command_count = sizeof(commands) / sizeof(commands[0]);

void api_init(void)
{
    profiling_init();
}

void api_update(void)
{
    profiling_iteration();
}

void api_handle(const command_t *command)
{
    if (command == NULL || command->name == NULL)
    {
        return;
    }

    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    for (size_t i = 0; i < api_command_count; i++)
    {
        if (strcmp(command->name, commands[i].name) == 0)
        {
            commands[i].callback(command);
            return;
        }
    }
    printf("error: unknown command '%s', try help\n", command->name);
}

static void cmd_help(const command_t *)
{
    for (size_t i = 0; i < api_command_count; i++)
    {
        printf("%-16s %s\n", commands[i].name, commands[i].help);
    }
}

void cmd_info(const command_t *)
{
    device_info();
}

void cmd_uptime(const command_t *)
{
    printf("uptime: %u ms\n", (uint32_t)(time_us_64() / 1000));
}

void cmd_led_enable(const command_t *)
{
    led_task_set_state(LED_STATE_ON);
    print_led_state();
}

void cmd_led_disable(const command_t *)
{
    led_task_set_state(LED_STATE_OFF);
    print_led_state();
}

void cmd_led_blink(const command_t *)
{
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

void cmd_led_period(const command_t *command)
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
    printf(", period %u ms\n", led_task_get_period_ms());
}

void cmd_button(const command_t *)
{
    printf("button: %s, presses %u\n",
           button_task_is_pressed() ? "pressed" : "released", button_task_get_press_count());
}

void cmd_pi_start(const command_t *command)
{
    uint32_t terms = pi_task_get_terms();

    if (command->argc >= 1)
    {
        if (command->argc > 1 || !parse_u32(command->argv[0], &terms))
        {
            printf("error: usage pi_start [<terms>]\n");
            return;
        }
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

    printf("pi: started, %u terms\n", pi_task_get_terms());
}

void cmd_pi(const command_t *)
{
    switch (pi_task_get_state())
    {
    case NOT_START:
        printf("pi: not started\n");
        break;
    case CALUTATE:
        printf("pi: running\n");
        break;
    case FINISH:
        printf("pi: %.9f in %u ms\n", pi_task_get_result(), pi_task_get_time_calculate());
        break;
    default:
        break;
    }
}

void cmd_profiling(const command_t *)
{
    printf("iteration avg %.2f us, max %" PRIu32 " us\n",
           profiling_avg_us(), profiling_max_us());
}

void cmd_profiling_reset(const command_t *)
{
    profiling_reset_max();
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