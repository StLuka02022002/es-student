#pragma once
#include "stdint.h"

#define DEFAULT_PI_TERMS 1000

typedef enum {
    NOT_START,
    CALUTATE,
    FINISH,
} pi_task_state_t;

void pi_task_init(void);

bool pi_task_start(void);

void pi_task_handle(void);

pi_task_state_t pi_task_get_state(void);

bool pi_task_set_terms(uint32_t terms);

uint32_t pi_task_get_terms(void);

double pi_task_get_result(void);

uint32_t pi_task_get_time_calculate(void);