#pragma once

#include <stdint.h>

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.1.0"
#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/StLuka02022002/es-student"

struct info_t
{
    uint32_t version;
    char name[13];
    uint8_t revision;
};

void device_info(void);

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif