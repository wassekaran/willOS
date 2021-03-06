#include "kmain.h"
#include <core/isr.h>
#include <core/timer.h>
#include <core/debug.h>
#include <core/check.h>
#include <core/cmos.h>
#include <mmu/mmu.h>
#include <kernel/panic.h>
#include <drivers/screen.h>
#include <drivers/serial.h>
#include <drivers/keyboard.h>
#include <stdio.h>

void print_welcome_messge()
{
    printf("%s\n", KERNEL_ASCII);
    printf("%s %s / Built on: %s at %s\n\n", KERNEL_NAME, KERNEL_VERSION, KERNEL_DATE, KERNEL_TIME);
}

void kmain(unsigned long magic, unsigned long addr)
{
    screen_init();
    screen_clear();

    if (multiboot_is_valid(magic, addr) == false) {
        PANIC("invalid multiboot");
    }

    multiboot_info_t *mbi = (multiboot_info_t*) addr;
    reserved_areas_t reserved = read_multiboot_info(mbi);

    print_welcome_messge();
    printf("- multiboot_start = 0x%X, multiboot_end = 0x%X\n", reserved.multiboot_start, reserved.multiboot_end);
    printf("- kernel_start    = 0x%X, kernel_end    = 0x%X\n", reserved.kernel_start, reserved.kernel_end);

    isr_init();
    irq_init();
    printf("- interruptions enabled\n");

    // enable scheduler
    timer_init(50); // 50hz
    printf("- clock (timer) enabled\n");

    // self-checks
    check_interrupts();

    // enable serial port
    serial_init(SERIAL_COM1, SERIAL_SPEED_115200);
    DEBUG("%s has started", KERNEL_NAME);

    keyboard_init();
    printf("- keyboard routine enabled\n");

    // memory
    multiboot_tag_mmap_t *mmap = find_multiboot_tag(mbi->tags, MULTIBOOT_TAG_TYPE_MMAP);
    mmu_init(mmap, reserved);
    printf("- frame allocator and paging enabled\n");

    // cmos
    cmos_rtc_t rtc = cmos_read_rtc();
    printf(
        "\nHello Human, today's date and time is %2d/%2d/%2d %2d:%2d:%2d UTC\n",
        rtc.year, rtc.month, rtc.day,
        rtc.hours, rtc.minutes, rtc.seconds
    );

    // prompt :p
    printf("$ ");

    while (1) {
        __asm__("hlt");
    }
}
