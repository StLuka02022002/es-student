#pragma once

#include <stdint.h>
#include "pico/unique_id.h"
#include "pico/version.h"

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.1.0"
#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/StLuka02022002/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

#define BOARD_ID_STR_LEN (PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1)

typedef struct device_info_t
{
    const char *name;
    const char *version;
    const char *project;
    const char *repo;
    const char *board;
    char board_id[BOARD_ID_STR_LEN];
    uint32_t manufacturer;
    uint32_t part;
    uint32_t revision;
    const char *pico_sdk;
} device_info_t;

void device_info(void);

void device_get_info(device_info_t *info);