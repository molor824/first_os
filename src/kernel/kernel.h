#ifndef __KERNEL_H
#define __KERNEL_H

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

typedef struct {
    uint32_t type;
    uint32_t size;
    uint8_t data[];
} boot_tag_t;
typedef struct {
    uint32_t total_size;
    uint32_t reserved;
    boot_tag_t tags[];
} __attribute__((packed)) boot_info_t;

extern uint32_t page_directory_entries[1024] __attribute__((aligned(4096)));
extern uint32_t page_tables_entries[1024 * 1024] __attribute__((aligned(4096)));
extern size_t kernel_end_addr;
extern boot_info_t *boot_info;
void *mmap(void *mem, size_t size);

#endif