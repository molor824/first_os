#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PDE_RANGE (1024 * 4096)
#define REFRESH_CR3() __asm__ volatile( \
    "mov %%cr3, %%eax\n" \
    "mov %%eax, %%cr3" \
    : : : "eax" \
)
#define FRAMEBUFFER_INFO 8
#define MEMORY_MAP_INFO 6

typedef struct {
    uint32_t type;
    uint32_t size;
} boot_tag_t;
typedef struct {
    uint32_t total_size;
    uint32_t reserved;
    boot_tag_t tags[];
} __attribute__((packed)) boot_info_t;

// _kernel_start is in physical addressing
extern char _kernel_start;
// _kernel_end is in virtual addressing
extern char _kernel_end;

extern char PDE_INDEX;
extern char KERNEL_ADDR;

// NOTE to self: This is fine, because each PDE points to 1024 PTE, which is exactly 4096 in length
// So no matter what this is aligned!
extern uint32_t page_directory_entries[1024];
extern uint32_t page_tables_entries[1024 * 1024];
extern size_t vmem_end_addr;
extern size_t kernel_end_addr;
extern boot_info_t *boot_info;
