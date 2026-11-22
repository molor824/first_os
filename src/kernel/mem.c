#include "mem.h"
#include "kernel.h"
#include "limine.h"
#include "stdio.h"
#include "string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define MAX_ORDER 10

typedef struct {
    uint32_t prev, next;
    uint16_t order;
    bool full;
} page_t;

static page_t *page_states;
static page_t *page_orders[MAX_ORDER + 1];
static uint64_t page_count;
static uint64_t mem_start, mem_end;

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

static struct limine_hhdm_response hhdm_response;
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

    mem_start = (largest_entry.base + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
    mem_end = (largest_entry.base + largest_entry.length) / PAGE_SIZE * PAGE_SIZE;
    page_count = (mem_end - mem_start) / PAGE_SIZE;
    page_states = hhdm_vaddr((void*)largest_entry.base);

    while (largest_entry.base + page_count * sizeof(page_t) > mem_start) {
        mem_start += PAGE_SIZE;
        page_count--;
        if (page_count == 0) {
            printf("ERROR: memory insufficient\n");
            flush(STDOUT);
            hcf();
        }
    }

    memset(page_states, 0, page_count * sizeof(page_t));

    printf("Total page count: %lu, page states: %p, mem_start: %p, mem_end: %p\n", page_count, page_states, (void*)mem_start, (void*)mem_end);
}

void *hhdm_vaddr(void *paddr) {
    return paddr + hhdm_response.offset;
}
void *hhdm_paddr(void *vaddr) {
    return vaddr - hhdm_response.offset;
}
void *phys_page_alloc() {
}
