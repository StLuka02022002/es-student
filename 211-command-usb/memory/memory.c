#include "memory.h"
#include <stdio.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "../command.h"
#include "device.h"
#include "string.h"
#include "led.h"
#include <stdlib.h>

int main(void);

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

uint32_t data_variable = 100;
uint32_t bss_variable;

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void fw_info()
{
    data_variable++;
    bss_variable++;
    uint32_t stack_variable = 1946;
    uint32_t *heap_variable = malloc(sizeof(uint32_t));

    uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
    uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    printf("%-10s %-8s %s\n", "object", "address", "value");
    printf("%-10s 0x%08x %04x\n", "main", (uintptr_t)main, *main_code);

    printf("%-10s 0x%08x %04x\n", "fw_info", (uintptr_t)fw_info, *fw_info_code);
    printf("%-10s 0x%08x\n", "commands", (uintptr_t)&commands);
    for (uint i = 0; i < command_count; i++)
    {
        char line[32];
        snprintf(line, sizeof(line), "- %s", commands[i].name);
        printf("%-10s 0x%08x\n", line, (uintptr_t)commands[i].handler);
    }

    printf("%-10s 0x%08x %s\n", "DEVICE_PROJECT", (uintptr_t)DEVICE_PROJECT, DEVICE_PROJECT);
    printf("%-10s 0x%08x %s\n", "DEVICE_BOARD", (uintptr_t)DEVICE_BOARD, DEVICE_BOARD);
    printf("%-10s 0x%08x %u\n", "data_variable", (uintptr_t)&data_variable, data_variable);
    printf("%-10s 0x%08x %u\n", "bss_variable", (uintptr_t)&bss_variable, bss_variable);
    printf("%-10s 0x%08x %u\n", "stack_variable", (uintptr_t)&stack_variable, stack_variable);
    printf("%-10s 0x%08x %u\n", "heap_variable", (uintptr_t)heap_variable, *heap_variable);

    free(heap_variable);
}

void mem_info(void)
{

    printf("%-10s %-8s %-8s %8s\n", "area", "start", "end", "size");

    unsigned ram = 16 * 1024;
    unsigned sram = 264 * 1024;

    row("flash", (uintptr_t)XIP_BASE, (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES));
    row("sram", (uintptr_t)SRAM_BASE, (uintptr_t)(SRAM_BASE + sram));
    row("rom", (uintptr_t)ROM_BASE, (uintptr_t)(ROM_BASE + ram));

    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free", (uintptr_t)&__flash_binary_end, (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES));
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);

    row("data flash", (uintptr_t)&__etext, (uintptr_t)&__flash_binary_end);
    row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
    row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    unsigned flash_image = (unsigned)(&__flash_binary_end - &__flash_binary_start);
    unsigned boot2 = (unsigned)(&__boot2_end__ - &__boot2_start__);
    unsigned text = (unsigned)(&__etext - &__boot2_end__);
    unsigned data = (unsigned)(&__data_end__ - &__data_start__);

    unsigned flash_free = (unsigned)((uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES) - ((uintptr_t)&__flash_binary_end));
    unsigned bss = (unsigned)(&__bss_end__ - &__bss_start__);
    unsigned ram_used = data + bss;
    unsigned heep = (unsigned)(&__HeapLimit - &__bss_end__);
    unsigned stack = (unsigned)(&__StackTop - &__StackBottom);

    printf("total\n");
    printf("  %-12s %9u = boot2 %u + text %u + data %u\n", "flash image", flash_image, boot2, text, data);
    printf("  %-12s %9u of %u\n", "flash free", flash_free, PICO_FLASH_SIZE_BYTES);
    printf("  %-12s %9u = data %u + bss %u\n", "ram used", ram_used, data, bss);
    printf("  %-12s %9u for heep and %u for stack\n", "ram free", heep, stack);
}

void boot_info(void)
{
    const uint32_t *vectors = (const uint32_t *)VECTOR_TABLE;

    uint32_t stack_top = vectors[0];
    uint32_t reset_handler = vectors[1];

    uint32_t *gpio_in = (uint32_t *)0xd0000004;
    uint32_t level = (*gpio_in >> led_pin()) & 1u;

    uint16_t *handler_code = (uint16_t *)((uintptr_t)reset_handler & ~1u);

    printf("%-13s 0x%08x\n", "vector table", vectors);
    printf("%-15s 0x%08x\n", "  stack top", (uintptr_t)stack_top);
    printf("%-15s 0x%08x\n", "  reset", (uintptr_t)reset_handler);
    printf("%-15s 0x%08x\n", "  reset (even)", handler_code);
    printf("%-13s 0x%08x\n", "gprio in", (uintptr_t)gpio_in);
    printf("%-15s %u\n", "  led bit", level);
    printf("%-15s %u\n", "  gptio_get", gpio_get(led_pin()));
}