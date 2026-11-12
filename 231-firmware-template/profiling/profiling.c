#include "profiling.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"

// итерация около 2 мкс, за секунду их около 500 000;
// ближайшая степень двойки 2^19 = 524 288, среднее помнит около секунды
#define AVG_SHIFT 19
// замер в 1/256 долях микросекунды, чтобы среднее не теряло дробную часть
#define FRACTION_SHIFT 8

static uint64_t avg_sum = 0;
static uint32_t previous_us = 0;
static uint32_t max_us = 0;

static profiling_result_t profiling_result[COUNT_PROFILING_STOPWATCH];
static uint32_t previus_us_all[COUNT_PROFILING_STOPWATCH];
static bool profiling_bisy[COUNT_PROFILING_STOPWATCH];

void profiling_init(void)
{
    previous_us = time_us_32();
    for (size_t i = 0; i < COUNT_PROFILING_STOPWATCH; i++)
    {
        previus_us_all[i] = time_us_32();
        profiling_bisy[i] = false;
    }
}

void profiling_iteration(void)
{
    uint32_t now_us = time_us_32();
    uint32_t iteration_us = now_us - previous_us;
    previous_us = now_us;
    if (iteration_us > max_us)
    {
        max_us = iteration_us;
    }

    avg_sum = avg_sum - (avg_sum >> AVG_SHIFT) + ((uint64_t)iteration_us << FRACTION_SHIFT);
}
float profiling_avg_us(void)
{
    return (float)(avg_sum >> AVG_SHIFT) / (1 << FRACTION_SHIFT);
}

uint32_t profiling_max_us(void)
{
    return max_us;
}
void profiling_reset_max(void)
{
    max_us = 0;
}

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id < COUNT_PROFILING_STOPWATCH)
    {
        profiling_result[id].name = name;
        profiling_result[id].avg_sum_us = 0;
        profiling_result[id].max_us = 0;
        profiling_bisy[id] = true;
    }
}

void profiling_start(uint32_t id)
{
    if (id < COUNT_PROFILING_STOPWATCH && profiling_bisy[id])
    {
        previus_us_all[id] = time_us_32();
    }
}
void profiling_stop(uint32_t id)
{
    if (id < COUNT_PROFILING_STOPWATCH && profiling_bisy[id])
    {
        uint32_t now_us = time_us_32();
        uint32_t iteration_us = now_us - previus_us_all[id];
        profiling_result[id].count++;
        if (iteration_us > profiling_result[id].max_us)
        {
            profiling_result[id].max_us = iteration_us;
        }
        profiling_result[id].avg_sum_us += iteration_us;
    }
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id < COUNT_PROFILING_STOPWATCH && profiling_bisy[id])
    {
        *result = profiling_result[id];
        return true;
    }
    return false;
}

void profiling_reset(void)
{
    for (size_t i = 0; i < COUNT_PROFILING_STOPWATCH; i++)
    {
        if (profiling_bisy[i])
        {
            profiling_result[i].avg_sum_us = 0;
            profiling_result[i].max_us = 0;
        }
    }
}