#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>
#include "gdt.h"

typedef struct {
    uint32_t vector;
    uint32_t edi, esi, ebp, _esp, ebx, edx, ecx, eax; // _esp in here is the esp before pushal instruction, meaning its out of date. access current one if needed
    uint32_t error;
    uint32_t eip, cs, eflags;
} int_registers_t;

typedef struct {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t isr_high;
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10)))
extern idt_entry_t idt_entries[256];

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;
extern idtr_t idtr;

void exception_handler(int_registers_t *);
void irq_handler(int_registers_t *);
void idt_init(void);

#endif
