#include "pi-task.h"
#include "stdint.h"
#include "pico/stdlib.h"
#include "systime/systime.h"

pi_task_state_t pi_task_state;

// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585 тактов;
// 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
// 0,47 мc околок 100 членов
static const uint32_t PI_TERMS_PER_CALL = 100;
static double result_pi = 0;
static double current_sum = 0.0;
static double current_sign = 1.0;
static uint32_t current_k = 0;
static uint64_t start_pi = 0;
static uint64_t end_pi = 0;

static uint32_t terms_pi;

void pi_task_init(void)
{
    pi_task_state = NOT_START;
    terms_pi = DEFAULT_PI_TERMS;
}

bool pi_task_set_terms(uint32_t terms)
{
    if (terms == NULL)
    {
        terms_pi = DEFAULT_PI_TERMS;
        return true;
    }
    if (terms > 0)
    {
        terms_pi = terms;
        return true;
    }
    return false;
}

void pi_task_handle(void)
{
    if (pi_task_state == CALUTATE)
    {
        uint32_t processed = 0;
        while (current_k < terms_pi && processed < PI_TERMS_PER_CALL)
        {
            current_sum += current_sign / (2.0 * (double)current_k + 1.0);
            current_sign = -current_sign;
            current_k++;
            processed++;
        }

        if (current_k >= terms_pi)
        {
            result_pi = 4.0 * current_sum;
            end_pi = systime_us();
            pi_task_state = FINISH;
        }
    }
}

bool pi_task_start(void)
{
    if (pi_task_state == CALUTATE)
    {
        return false;
    }

    pi_task_state = CALUTATE;
    result_pi = 0;
    current_sum = 0.0;
    current_sign = 1.0;
    current_k = 0;
    start_pi = systime_us();
    end_pi = 0;
    return true;
}

pi_task_state_t pi_task_get_state(void)
{
    return pi_task_state;
}

uint32_t pi_task_get_terms(void)
{
    return terms_pi;
}

double pi_task_get_result(void)
{
    if(pi_task_state == NOT_START){
        return -1.0;
    }
    return result_pi;
}

uint32_t pi_task_get_time_calculate(void){
    if(pi_task_state == FINISH){
        return (uint32_t)((end_pi - start_pi) / 1000);
    }
    return 0;
}