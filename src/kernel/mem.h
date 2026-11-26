#pragma once

#include <stddef.h>
#include <stdint.h>
#include "limine.h"

#define PAGE_SIZE 0x1000

void mem_init();

uintptr_t phys_page_alloc();
int phys_page_free(uintptr_t ppage);

extern struct limine_hhdm_response hhdm_response;

static inline uintptr_t hhdm_vaddr(uintptr_t paddr) {
    return paddr + hhdm_response.offset;
}
static inline uintptr_t hhdm_paddr(uintptr_t vaddr) {
    return vaddr - hhdm_response.offset;
}
static inline uintptr_t align_down(uintptr_t address, size_t alignment) {
    return address / alignment * alignment;
}
static inline uintptr_t align_up(uintptr_t address, size_t alignment) {
    return align_down(address + alignment - 1, alignment);
}
