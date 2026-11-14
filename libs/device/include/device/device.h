#pragma once

#include <stdint.h>

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

#define BOARD_ID_STR_LEN 17

typedef struct device_info_t
{
    const char *board;
    char board_id[BOARD_ID_STR_LEN];
    uint32_t manufacturer;
    uint32_t part;
    uint32_t revision;
    const char *pico_sdk;
} device_info_t;

void device_get_info(device_info_t *info);