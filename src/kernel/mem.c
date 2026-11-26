#include "mem.h"
#include "kernel.h"
#include "limine.h"
#include "stdio.h"
#include "string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_FREE 0
#define PAGE_ALLOC 1

static uint8_t *page_states;
static uint32_t *page_next_free;
static uint32_t page_free_root;
static size_t page_count;
static uintptr_t pmem_start, pmem_end;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};
__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

struct limine_hhdm_response hhdm_response;
static struct limine_memmap_response memmap_response;
static struct limine_memmap_entry largest_entry;

void mem_init() {
    memmap_response = *memmap_request.response;
    hhdm_response = *hhdm_request.response;
    struct limine_memmap_entry *p_largest_entry = NULL;

    for (size_t i = 0; i < memmap_response.entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response.entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;
        if (!p_largest_entry || p_largest_entry->length < entry->length)
            p_largest_entry = entry;

        printf("Usable memory base: %p, length: %lu\n", (void*)entry->base, entry->length);
    }

    if (!p_largest_entry) {
        printf("ERROR: Failed to find available memmap!\n");
        flush(STDOUT);
        hcf();
    }

    largest_entry = *p_largest_entry;
    printf("Largest usable memory base: %p, length: %lu\n", (void*)largest_entry.base, largest_entry.length);
    printf("HHDM offset: %lx\n", hhdm_response.offset);

    pmem_start = align_up(largest_entry.base, PAGE_SIZE);
    pmem_end = align_down(largest_entry.base + largest_entry.length, PAGE_SIZE);
    page_count = (pmem_end - pmem_start) / PAGE_SIZE;

    uintptr_t page_states_phys = largest_entry.base;
    uintptr_t page_next_free_phys = align_up(page_states_phys + page_count, _Alignof(uint32_t));

    page_states = (void*)hhdm_vaddr(page_states_phys);
    page_next_free = (void*)hhdm_vaddr(page_next_free_phys);

    uintptr_t state_end_phys = page_next_free_phys + page_count * sizeof(uint32_t);

    if (state_end_phys > pmem_start) {
        uintptr_t diff = state_end_phys - pmem_start;
        size_t count = (diff + PAGE_SIZE - 1) / PAGE_SIZE;
        page_count -= count;
        pmem_start += count * PAGE_SIZE;
    }

    memset(page_states, PAGE_FREE, page_count * sizeof(uint8_t));
    for (size_t i = 0; i < page_count; i++) {
        page_next_free[i] = i + 1 < page_count ? i + 1 : UINT32_MAX;
    }

    printf("Total page count: %lu, pmem_start: %p, pmem_end: %p\n", page_count, (void*)pmem_start, (void*)pmem_end);
}

uintptr_t phys_page_alloc() {
    if (page_free_root == UINT32_MAX) return 0;
    uint32_t idx = page_free_root;
    if (idx >= page_count) return 0; // safety
    page_states[idx] = PAGE_ALLOC;
    page_free_root = page_next_free[idx];
    return pmem_start + idx * PAGE_SIZE;
}

int phys_page_free(uintptr_t ppage) {
    if (ppage < pmem_start) return -1;
    if ((ppage - pmem_start) % PAGE_SIZE != 0) return -1;
    uint32_t idx = (ppage - pmem_start) / PAGE_SIZE;
    if (idx >= page_count) return -1;
    if (page_states[idx] != PAGE_ALLOC) return -1;
    page_states[idx] = PAGE_FREE;
    page_next_free[idx] = page_free_root;
    page_free_root = idx;
    return 0;
}
