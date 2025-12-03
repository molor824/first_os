#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include <stdbool.h>

idt_entry_t idt_entries[256];
idtr_t idtr;
bool int_vectors[256];
extern void *isr_stub_table[32];

void exception_handler(void) {
    printf("EXCEPTION");
    fflush(STDOUT);
    for (;;)
        __asm__ volatile("cli\nhlt");
}

void idt_lidt(void);
void idt_set_descriptor(uint8_t, void *, uint8_t);
__attribute__((constructor(IDT_CRT_PRIORITY))) void gdt_crt(void) {
    for (int vector = 0; vector < sizeof(isr_stub_table) / sizeof(void*); vector++) {
        // 0x8E is 32-bit interrupt gate (see https://wiki.osdev.org/Interrupt_Descriptor_Table#Structure_on_x86-64)
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        int_vectors[vector] = true;
    }
    idt_lidt();
    printf("IDT setup complete.\n");
    fflush(STDOUT);
}

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
    idt_entries[vector] = (idt_entry_t){
        .isr_low = (uint32_t)isr,
        .isr_high = (uint32_t)isr >> 16,
        .kernel_cs = KERNEL_CODE_SEGMENT * sizeof(uint64_t),
        .attributes = flags,
        .reserved = 0,
    };
}

void idt_lidt(void) {
    idtr.base = (uint32_t)idt_entries;
    idtr.limit = (uint16_t)(sizeof(idt_entries) - 1);

    __asm__ volatile("lidt %0\nsti" : : "m"(idtr));
}
