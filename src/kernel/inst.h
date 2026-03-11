#ifndef __INST_H
#define __INST_H

// Definition of common instructions

#include <stdint.h>

#define cli() __asm__ volatile("cli")
#define sti() __asm__ volatile("sti")
#define hlt() __asm__ volatile("hlt")

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif