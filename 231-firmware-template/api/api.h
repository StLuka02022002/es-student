#pragma once

#include <stdint.h>
#include "stdio_text_protocol/stdio_text_protocol.h"

typedef void (*api_callback_t)(const command_t *command);

typedef struct 
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

void api_init();

void api_handle(const command_t *command);

void api_update(void);

