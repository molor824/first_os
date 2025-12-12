#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>
#include "gdt.h"

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

void exception_handler(int index);
void irq_handler(int index);
void idt_init(void);

#endif