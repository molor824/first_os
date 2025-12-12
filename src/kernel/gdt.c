#include "gdt.h"
#include "file.h"
#include "stdio.h"

uint64_t gdt_entries[6];

static uint64_t gdt_entry(gdt_params_t params);
static void load_gdt(void);

void gdt_init(void) {
    // Kernel mode code segment
    gdt_entries[KERNEL_CODE_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0xFFFFF,
        .access_byte = 0x9A, // Present, Ring 0, Code
        .flags = 0xC       // 4K granularity, 32-bit
    });

    // Kernel mode data segment
    gdt_entries[KERNEL_DATA_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0xFFFFF,
        .access_byte = 0x92, // Present, Ring 0, Data
        .flags = 0xC       // 4K granularity, 32-bit
    });

    // User mode code segment
    gdt_entries[USER_CODE_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0xFFFFF,
        .access_byte = 0xFA, // Present, Ring 3, Code
        .flags = 0xC       // 4K granularity, 32-bit
    });

    // User mode data segment
    gdt_entries[USER_DATA_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0xFFFFF,
        .access_byte = 0xF2, // Present, Ring 3, Data
        .flags = 0xC         // 4K granularity, 32-bit
    });

    // Task state segment PLACEHOLDER
    gdt_entries[TASK_STATE_SEGMENT] = gdt_entry((gdt_params_t){
        .base = 0,
        .limit = 0,
        .access_byte = 0x89, // Present, Ring 0, TSS
        .flags = 0x0         // byte granularity
    });

    load_gdt();

    printf("GDT setup complete.\n");
}

static uint64_t gdt_entry(gdt_params_t params) {
    return (uint64_t)(params.limit & 0xFFFF) |       // limits[15:0]
        (uint64_t)(params.base & 0xFFFFFF) << 16 |   // base[23:0]
        (uint64_t)params.access_byte << 40 |         // access byte
        (uint64_t)(params.limit >> 16 & 0xF) << 48 | // limits[19:16]
        (uint64_t)(params.flags & 0xF) << 52 |       // flags
        (uint64_t)(params.base >> 24 & 0xFF) << 56;  // base[31:24]
}

static void load_gdt(void) {
    uint32_t base = (uint32_t)gdt_entries;
    uint16_t gdt_ptr[3];
    gdt_ptr[0] = sizeof(gdt_entries) - 1;        // Limit
    gdt_ptr[1] = base;         // Base low
    gdt_ptr[2] = base >> 16; // Base high

    __asm__ volatile (
        "cli\n"
        "lgdt %0\n"
        : : "m"(gdt_ptr)
    );

    // For GDT to take effect, the registers must be updated
    // However CS cannot be changed directly, so we use a far jump
    __asm__ volatile (
        "mov $0x10, %ax\n"
        "mov %ax, %ds\n"
        "mov %ax, %es\n"
        "mov %ax, %fs\n"
        "mov %ax, %gs\n"
        "mov %ax, %ss\n"
        "ljmp $0x08, $.1\n"
        ".1:\n"
        "sti\n"
    );
}
