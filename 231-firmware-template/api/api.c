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
#include "led.h"

#define BUTTON_PIN 15
#define DEBOUNCE_MS 5
#define DEFAULT_PI_TERMS 1000
#define PI_CHUNK_TERMS 100

typedef enum
{
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_BLINK,
} led_state_t;

static led_state_t led_state = LED_STATE_OFF;
static uint64_t period_us = 1000 * 1000;
static uint64_t last_toggle_us = 0;

static int32_t button_count = 0;
static bool button_prev = true;

static bool running_pi = false;
static uint32_t terms_pi = DEFAULT_PI_TERMS;
static double result_pi = -1.0;
static double current_sum = 0.0;
static double current_sign = 1.0;
static uint32_t current_k = 0;
static uint64_t start_pi = 0;
static uint64_t end_pi = 0;

static void cmd_help(const command_t *);
static void print_led_state(void);
static void blink(void);
static void calc_pi(void);
static bool parse_u32(const char *text, uint32_t *value);
static bool get_button_pressed(void);

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
    led_init();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    button_prev = gpio_get(BUTTON_PIN);

    led_set(false);
    led_state = LED_STATE_OFF;

    profiling_init();
}

void api_update(void)
{
    if (led_state == LED_STATE_BLINK)
    {
        blink();
    }

    if (get_button_pressed())
    {
        button_count++;
    }

    if (running_pi)
    {
        calc_pi();
    }

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
    printf("uptime: %u ms\n", time_us_64() / 1000);
}

void cmd_led_enable(const command_t *)
{
    led_state = LED_STATE_ON;
    led_set(true);
    print_led_state();
}

void cmd_led_disable(const command_t *)
{
    led_state = LED_STATE_OFF;
    led_set(false);
    print_led_state();
}

void cmd_led_blink(const command_t *)
{
    led_state = LED_STATE_BLINK;
    last_toggle_us = 0;
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
    if (period_ms == 0)
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    period_us = (uint64_t)period_ms * 1000;
    print_led_state();
}

static void print_led_state(void)
{
    printf("led: ");
    switch (led_state)
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
    printf(", period %u ms\n", (uint32_t) period_us / 1000);
}

static void blink(void)
{
    uint64_t now_us = time_us_64();
    if (now_us - last_toggle_us >= period_us)
    {
        last_toggle_us = now_us;
        led_toggle();
    }
}

static bool get_button_pressed(void)
{
    static uint64_t last_change_us = 0;
    static bool stable_state = true;
    static bool reported = false;

    bool raw = gpio_get(BUTTON_PIN);
    uint64_t now = time_us_64();

    if (raw != stable_state)
    {
        last_change_us = now;
        stable_state = raw;
        reported = false;
        return false;
    }

    if (!reported && (now - last_change_us) >= (uint64_t)DEBOUNCE_MS * 1000)
    {
        reported = true;
        if (stable_state == false && button_prev == true)
        {
            button_prev = false;
            return true;
        }
        button_prev = stable_state;
    }
    return false;
}

void cmd_button(const command_t *)
{
    bool pressed = (gpio_get(BUTTON_PIN) == 0);
    printf("button: %s, presses %" PRId32 "\n",
           pressed ? "pressed" : "released", button_count);
}

void cmd_pi_start(const command_t *command)
{
    uint32_t terms = DEFAULT_PI_TERMS;

    if (running_pi)
    {
        printf("error: pi is busy\n");
        return;
    }

    if (command->argc >= 1)
    {
        if (command->argc > 1 || !parse_u32(command->argv[0], &terms))
        {
            printf("error: usage pi_start [<terms>]\n");
            return;
        }
    }
    if (terms == 0)
    {
        printf("error: terms must be greater than 0\n");
        return;
    }

    terms_pi = terms;
    current_sum = 0.0;
    current_sign = 1.0;
    current_k = 0;
    result_pi = -1.0;
    start_pi = time_us_64();
    end_pi = 0;
    running_pi = true;

    printf("pi: started, %" PRIu32 " terms\n", terms);
}

void cmd_pi(const command_t *)
{
    if (running_pi)
    {
        printf("pi: running\n");
        return;
    }

    if (result_pi < 0)
    {
        printf("pi: not started\n");
        return;
    }

    printf("pi: %.9f in %u ms\n", result_pi, (end_pi - start_pi) / 1000);
}

static void calc_pi(void)
{
    uint32_t processed = 0;

    while (current_k < terms_pi && processed < PI_CHUNK_TERMS)
    {
        current_sum += current_sign / (2.0 * (double)current_k + 1.0);
        current_sign = -current_sign;
        current_k++;
        processed++;
    }

    if (current_k >= terms_pi)
    {
        result_pi = 4.0 * current_sum;
        end_pi = time_us_64();
        running_pi = false;
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