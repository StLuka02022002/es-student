#include "stdio-text-protocol/stdio-text-protocol.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

static char buffer[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static size_t position = 0;
static bool line_truncated = false;
static command_t command;

void stdio_text_protocol_init(void)
{
    stdio_init_all();
    position = 0;
    line_truncated = false;
    memset(&command, 0, sizeof(command));
}

static const command_t *parse_command(void)
{
    command.argc = 0;
    command.truncated = false;
    command.name = NULL;

    if (line_truncated)
    {
        command.truncated = true;
        line_truncated = false;
        return &command;
    }

    size_t i = 0;

    while (i < position && buffer[i] == ' ')
        i++;

    if (i >= position)
        return NULL;

    command.name = &buffer[i];

    while (i < position && buffer[i] != ' ')
        i++;

    if (i < position)
        buffer[i++] = '\0';

    while (i < position)
    {
        while (i < position && buffer[i] == ' ')
            i++;

        if (i >= position)
            break;

        if (command.argc >= STDIO_TEXT_PROTOCOL_MAX_ARGS)
        {
            command.truncated = true;
            break;
        }

        command.argv[command.argc++] = &buffer[i];

        while (i < position && buffer[i] != ' ')
            i++;

        if (i < position)
            buffer[i++] = '\0';
    }

    return &command;
}

const command_t *stdio_text_protocol_handle(void)
{
    int symbol = getchar_timeout_us(0);

    if (symbol == PICO_ERROR_TIMEOUT)
        return NULL;

    if (symbol == '\r' || symbol == '\n')
    {
        if (position == 0 && !line_truncated)
            return NULL;

        buffer[position] = '\0';
        printf("\r\n");

        const command_t *cmd = parse_command();
        position = 0;

        return cmd;
    }

    if (symbol == '\b' || symbol == 0x7f)
    {
        if (position > 0)
        {
            position--;
            printf("\b \b");
        }

        return NULL;
    }

    if (position >= STDIO_TEXT_PROTOCOL_LINE_SIZE - 1)
    {
        line_truncated = true;
        return NULL;
    }

    buffer[position++] = (char)symbol;
    putchar(symbol);

    return NULL;
}