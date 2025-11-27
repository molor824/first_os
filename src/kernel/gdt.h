#ifndef __GDT_H
#define __GDT_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flags;
} gdt_params_t;

uint64_t gdt_entry(gdt_params_t params);
void load_gdt(size_t size, void *base);

extern uint64_t entries[6];

#endif