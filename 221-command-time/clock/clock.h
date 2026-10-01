#pragma once
#include "pico/stdlib.h"

void clk_info(void);

void uptime(void);

static void clk_sys_set(uint32_t kHz);

void clk_sys_low(void);

void clk_sys_default(void);