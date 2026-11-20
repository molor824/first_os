#include "gdt.h"
#include "stdio.h"
#include <stdint.h>

uint64_t gdt_entries[7];

static uint8_t rsp0_stack[0x10000] __attribute__((aligned(16)));
static tss_t tss = {
    .rsp[0] = (uintptr_t)rsp0_stack + sizeof(rsp0_stack),
    .iopb = sizeof(tss_t),
};

void gdt_init(void) {
    // Null segment
    gdt_entries[0] = 0;

    // Kernel mode code segment
    gdt_entries[KERNEL_CODE_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0xFFFFF,
        .access_byte = ACCESS_P | ACCESS_S | ACCESS_E | ACCESS_RW,
        .flags = FLAG_L | FLAG_G
    });

    // Kernel mode data segment
    gdt_entries[KERNEL_DATA_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0xFFFFF,
        .access_byte = ACCESS_P | ACCESS_S | ACCESS_RW,
        .flags = FLAG_L | FLAG_G
    });

    // User mode code segment
    gdt_entries[USER_CODE_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0xFFFFF,
        .access_byte = ACCESS_P | ACCESS_S | ACCESS_RW | ACCESS_E | ACCESS_PRIV3,
        .flags = FLAG_L | FLAG_G
    });

    // User mode data segment
    gdt_entries[USER_DATA_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0xFFFFF,
        .access_byte = ACCESS_P | ACCESS_S | ACCESS_RW | ACCESS_PRIV3,
        .flags = FLAG_L | FLAG_G
    });

    // Task state segment PLACEHOLDER
    gdt_entries[TASK_STATE_SEGMENT] = gdt_entry((gdt_params_t){
        .base = (uint32_t)(uintptr_t)&tss,
        .limit = sizeof(tss_t) - 1,
        .access_byte = ACCESS_A | ACCESS_P | ACCESS_E,
        .flags = 0x0
    });
    gdt_entries[TASK_STATE_SEGMENT + 1] = (uintptr_t)&tss >> 32 & 0xFFFFFFFF;

    set_gdt(sizeof(gdt_entries) - 1, (uintptr_t)gdt_entries);
    reload_segments();

    // load TSS
    asm volatile("ltr %0" :: "r"((uint16_t)(TASK_STATE_SEGMENT * 8)));

    printf("GDT setup complete.\n");
}

uint64_t gdt_entry(gdt_params_t params) {
    return (uint64_t)(params.limit & 0xFFFF) |       // limits[15:0]
        (uint64_t)(params.base & 0xFFFFFF) << 16 |   // base[23:0]
        (uint64_t)params.access_byte << 40 |         // access byte
        (uint64_t)(params.limit >> 16 & 0xF) << 48 | // limits[19:16]
        (uint64_t)(params.flags & 0xF) << 52 |       // flags
        (uint64_t)(params.base >> 24 & 0xFF) << 56;  // base[31:24]
}
