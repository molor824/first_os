#ifndef __PIC_H
#define __PIC_H

#include <stdint.h>
#include <stdbool.h>
#include "idt.h"

#define MASTER_VECTOR_OFFSET 0x20
#define SLAVE_VECTOR_OFFSET 0x28

void PIC_sendEOI(uint8_t irq);
void IRQ_set_mask(uint8_t IRQ_line);
void IRQ_clear_mask(uint8_t IRQ_line);
void PIC_remap(int offset1, int offset2);
void PIC_disable(void);
uint16_t PIC_get_irr(void);
uint16_t PIC_get_isr(void);
void PIC_init(void);

extern void (*IRQ_callbacks[16])(void);

#endif