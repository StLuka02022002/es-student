#pragma once

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_PROJECT "134-led-module"
#define DEVICE_REPO "https://github.com/StLuka02022002/es-student"

void device_info(void);

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif