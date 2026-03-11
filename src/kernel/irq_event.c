#include "irq_event.h"
#include "inst.h"

static irq_event_t queue_buffer[IRQ_QUEUE_CAPACITY];
static volatile int queue_start;
static volatile int queue_size;

int irq_event_push(irq_event_t src) {
    int index = (queue_start + queue_size) % IRQ_QUEUE_CAPACITY;
    queue_buffer[index] = src;
    queue_size++;
    if (queue_size > IRQ_QUEUE_CAPACITY) {
        queue_size = IRQ_QUEUE_CAPACITY;
        queue_start = (queue_start + 1) % IRQ_QUEUE_CAPACITY;
        return 0;
    }
    return 1;
}
int irq_event_pop(irq_event_t *dst) {
    if (queue_size <= 0) return 0;
    if (dst) *dst = queue_buffer[queue_start];
    cli();
    queue_size--;
    queue_start = (queue_start + 1) % IRQ_QUEUE_CAPACITY;
    sti();
    return 1;
}