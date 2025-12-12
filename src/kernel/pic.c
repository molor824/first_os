#include <stdint.h>
#include "pic.h"
#include "io.h"
#include "stdio.h"

#define MASTER_PIC_CMD 0x20
#define MASTER_PIC_DAT 0x21
#define SLAVE_PIC_CMD 0xA0
#define SLAVE_PIC_DAT 0xA1

#define PIC_EOI 0x20

// Reinitialize PIC controllers, giving them specified vector offsets
// rather than 8h and 70h, as configured by default
// We need to initialize the two PICs with a command
// This command makes PIC wait for 3 extra initialization words
// ICW1: Initialize command
// ICW2: Vector offset
// ICW3: Master/slave wiring
// ICW4: Additional information about environment

#define ICW1_ICW4 0x01 // Indicates that ICW4 will be present
#define ICW1_SINGLE 0x02 // Single (cascade) mode
#define ICW1_INTERVAL4 0x04 // Call address interval 4
#define ICW1_LEVEL 0x08 // Level triggered (edge) mode
#define ICW1_INIT 0x10 // Initialization, required!

#define ICW4_8086 0x01 // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO 0x02 // Auto (normal) EOI
#define ICW4_BUF_SLAVE 0x08 // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C // Buffered mode/master
#define ICW4_SFNM 0x10 // Special fully nested (not)

#define CASCADE_IRQ 2

// PIC has 2 registers, ISR (In-Service Regster) and IRR (Interrupt Request Register).
// ISR tells us which interrupts are being serviced
// IRR tells us which interrupts have been raised
// ISR and IRR can be read via OCW3 command word. This is a command sent to one of the comand ports with bit 3 set.
// To read ISR or IRR, write the appropriate command to the command port, and then read the command port (NOT THE DATA PORT!).
#define PIC_READ_IRR 0x0A // OCW3 
#define PIC_READ_ISR 0x0B

#define outb_and_wait(port, value) \
    outb(port, value); \
    io_wait()

void PIC_init(void) {
    PIC_remap(MASTER_VECTOR_OFFSET, SLAVE_VECTOR_OFFSET);
    PIC_disable();
    for (int i = 0; i < 16; i++) {
        if (IRQ_callbacks[i])
            IRQ_clear_mask(i);
    }
}

static void IRQ_timer(void) {
    fflush(STDOUT);
}

#define KBD_DATA 0x60
#define KBD_STATUS 0x64

static void IRQ_keyboard(void) {
    uint8_t scancode = inb(KBD_DATA);
    
    if (scancode & 0x80) {
        printf("Released %d\n", scancode & ~0x80);
    } else {
        printf("Pressed %d\n", scancode);
    }
}

void (*IRQ_callbacks[16])(void) = {
    [0] = &IRQ_timer,
    [1] = &IRQ_keyboard,
};

static uint16_t __pic_get_irq_reg(int ocw3) {
    outb(MASTER_PIC_CMD, ocw3);
    outb(SLAVE_PIC_CMD, ocw3);
    return (inb(SLAVE_PIC_CMD) << 8) | inb(MASTER_PIC_CMD);
}
uint16_t PIC_get_irr(void) {
    return __pic_get_irq_reg(PIC_READ_IRR);
}
uint16_t PIC_get_isr(void) {
    return __pic_get_irq_reg(PIC_READ_ISR);
}

// offset1 - Vector offset for master PIC (offset1..offset1+7)
// offset2 - Vector offset for slave PIC (offset2..offset2+7)
void PIC_remap(int offset1, int offset2) {
    outb_and_wait(MASTER_PIC_CMD, ICW1_INIT | ICW1_ICW4); // Starts initialization sequence in cascade mode
    outb_and_wait(SLAVE_PIC_CMD, ICW1_INIT | ICW1_ICW4);
    outb_and_wait(MASTER_PIC_DAT, offset1); // ICW2: Master PIC Vector offset
    outb_and_wait(SLAVE_PIC_DAT, offset2); // ICW2: Slave PIC Vector offset
    outb_and_wait(MASTER_PIC_DAT, 1 << CASCADE_IRQ); // ICW3: tell Master PIC that there is a slave PIC at IRQ2
    outb_and_wait(SLAVE_PIC_DAT, 2); // ICW3: tell slave PIC it's cascade identity

    outb_and_wait(MASTER_PIC_CMD, ICW4_8086); // ICW4: have the PICs use 8086 mode
    outb_and_wait(SLAVE_PIC_CMD, ICW4_8086);
}

void PIC_disable(void) {
    outb(MASTER_PIC_DAT, 0xff);
    outb(SLAVE_PIC_DAT, 0xff);
}
void IRQ_set_mask(uint8_t IRQ_line) {
    uint16_t port;
    if (IRQ_line < 8) {
        port = MASTER_PIC_DAT;
    } else {
        port = SLAVE_PIC_DAT;
        IRQ_line -= 8;
    }

    outb(port, inb(port) | (1 << IRQ_line));
}
void IRQ_clear_mask(uint8_t IRQ_line) {
    uint16_t port;

    if (IRQ_line < 8) {
        port = MASTER_PIC_DAT;
    } else {
        port = SLAVE_PIC_DAT;
        IRQ_line -= 8;
    }

    outb(port, inb(port) & ~(1 << IRQ_line));
}

void PIC_sendEOI(uint8_t irq) {
    // Each PIC has 8 IRQs, and 0-7 are handled by master, while the rest is handled by the slave PIC
    // In such cases, both master and slave PIC must have PIC_EOI signal sent
    if (irq >= 8) 
        outb(SLAVE_PIC_CMD, PIC_EOI);
    outb(MASTER_PIC_CMD, PIC_EOI);
}
