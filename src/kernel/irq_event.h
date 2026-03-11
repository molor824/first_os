#ifndef __IRQ_EVENT_H
#define __IRQ_EVENT_H

#include <stdint.h>

#define IRQ_QUEUE_CAPACITY 0x1000

typedef enum irq_event_type {
    IRQ_EVENT_TIM,
    IRQ_EVENT_KBD
} irq_event_type_t;
typedef struct irq_event {
    irq_event_type_t type;
    union {
        uint8_t scancode;
    };
} irq_event_t;

// Returns: 0 - success, but overwritten first element, 1 - success
int irq_event_push(irq_event_t src);

// Returns: 0 - fail, 1 - success
int irq_event_pop(irq_event_t* dst);

#endif