#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include "pic.h"
#include <stdbool.h>

extern void *isr_stub_table;
extern void *isr_stub_table_end;

idt_entry_t idt_entries[256];
idtr_t idtr;

static bool idt_vectors[256];

void exception_handler(int index) {
    printf("EXCEPTION: %d\n", index);
    fflush(STDOUT);
    for (;;) {
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}
void irq_handler(int index) {
    size_t irq = index - MASTER_VECTOR_OFFSET;
    IRQ_callbacks[irq]();
    PIC_sendEOI(irq);
}

static void idt_lidt(void);
static void idt_set_descriptor(uint8_t, void *, uint8_t);

void idt_init(void) {
    for (size_t vector = 0; &(&isr_stub_table)[vector] < &isr_stub_table_end; vector++) {
        if (vector >= MASTER_VECTOR_OFFSET && vector < MASTER_VECTOR_OFFSET + 16 && IRQ_callbacks[vector - MASTER_VECTOR_OFFSET] == NULL) {
            continue;
        }
        // 0x8E is 32-bit interrupt gate (see https://wiki.osdev.org/Interrupt_Descriptor_Table#Structure_on_x86-64)
        idt_set_descriptor(vector, (&isr_stub_table)[vector], 0x8E);
        idt_vectors[vector] = true;
        printf("IDT vector set: %x\n", vector);
    }
    idt_lidt();
    printf("IDT setup complete.\n");
}

static void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
    idt_entries[vector] = (idt_entry_t){
        .isr_low = (uint32_t)isr,
        .isr_high = (uint32_t)isr >> 16,
        .kernel_cs = KERNEL_CODE_SEGMENT * sizeof(uint64_t),
        .attributes = flags,
        .reserved = 0,
    };
}

static void idt_lidt(void) {
    idtr.base = (uint32_t)idt_entries;
    idtr.limit = (uint16_t)(sizeof(idt_entries) - 1);

    __asm__ volatile("lidt %0" : : "m"(idtr));
    __asm__ volatile("sti");
}
