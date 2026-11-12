#pragma once

#include "pico/types.h"

#define COUNT_PROFILING_STOPWATCH 5

typedef struct profiling_result_t{
    const char *name;
    uint32_t count;
    // сумма замеров одного секундомера переполнится только через 71 минуту
    uint32_t avg_sum_us;
    uint32_t max_us;
} profiling_result_t;

void profiling_init(void);
void profiling_iteration(void);
float profiling_avg_us(void);
uint32_t profiling_max_us(void);
void profiling_reset_max(void);

void profiling_stopwatch_init(uint32_t id, const char *name);

void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);

bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);