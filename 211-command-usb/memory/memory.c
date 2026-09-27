#include "memory.h"
#include <stdio.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"

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

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
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