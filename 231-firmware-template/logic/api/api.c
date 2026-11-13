#include "api.h"
#include "api-commands.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void cmd_help(const command_t *);

static api_command_t help_command = {"help", "перечень команд со справкой", cmd_help};

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

    if (strcmp(command->name, help_command.name) == 0)
    {
        help_command.callback(command);
        return;
    }

    for (size_t i = 0; i < api_command_count; i++)
    {
        if (strcmp(command->name, api_commands[i].name) == 0)
        {
            api_commands[i].callback(command);
            return;
        }
    }
    printf("error: unknown command '%s', try help\n", command->name);
}

static void cmd_help(const command_t *command)
{
    printf("%-16s %s\n", help_command.name, help_command.help);
    for (size_t i = 0; i < api_command_count; i++)
    {
        printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
    }
}